#include "gtx8_brl.h"

#define TS_KIT_BASE_DECIMAL 10
#define NOCODE_LEN 6

struct fw_update_ctrl brl_update_ctrl = {
	.status = UPSTA_NOTWORK,
	.progress = FW_UPDATE_0_PERCENT,
	.force_update = false,
};

static int brl_reg_write(u32 addr, u8 *data, u32 len)
{
	return brl_ts->ops.i2c_write(addr, data, len);
}

static int brl_reg_read(u32 addr, u8 *data, u32 len)
{
	return brl_ts->ops.i2c_read(addr, data, len);
}


/* sysfs attributes */

/**
 * brl_request_firmware - request firmware data from user space
 *
 * @fw_data: firmware struct, contains firmware header info
 *	and firmware data pointer.
 * return: 0 - OK, < 0 - error
 */
static int brl_request_firmware(struct firmware_data *fw_data,
				const char *name)
{
	int ret;
	struct device *dev = &brl_ts->pdev->dev;

	TS_LOG_INFO("Request firmware image [%s]\n", name);
	ret = request_firmware(&fw_data->firmware, name, dev);
	if (ret < 0)
		TS_LOG_ERR("Firmware image [%s] not available,errno:%d\n",
			name, ret);
	else
		TS_LOG_INFO("Firmware image [%s] is ready\n", name);

	return ret;
}

/**
 * relase firmware resources
 *
 */
static void brl_release_firmware(struct firmware_data *fw_data)
{
	if (fw_data->firmware) {
		release_firmware(fw_data->firmware);
		fw_data->firmware = NULL;
	}
}


#define FW_FILE_CHECKSUM_OFFSET	8
#define BUS_TYPE_SPI	1
#define BUS_TYPE_I2C	0
/**
 * brl_parse_firmware - parse firmware header infomation
 *	and subsystem infomation from firmware data buffer
 *
 * @fw_data: firmware struct, contains firmware header info
 *	and firmware data.
 * return: 0 - OK, < 0 - error
 */
static int brl_parse_firmware(struct firmware_data *fw_data)
{
	const struct firmware *firmware;
	struct  firmware_info *fw_info;
	unsigned int i, fw_offset, info_offset;
	u32 checksum;
	int r = 0;

	fw_info = &fw_data->fw_info;

	/* copy firmware head info */
	firmware = fw_data->firmware;
	if (firmware->size < FW_SUBSYS_INFO_OFFSET) {
		TS_LOG_ERR("Invalid firmware size:%zu", firmware->size);
		r = -EINVAL;
		goto err_size;
	}
	memcpy(fw_info, firmware->data, sizeof(*fw_info));

	/* check firmware size */
	fw_info->size = le32_to_cpu(fw_info->size);
	if (firmware->size != fw_info->size + FW_FILE_CHECKSUM_OFFSET) {
		TS_LOG_ERR("Bad firmware, size not match, %zu != %d",
			firmware->size, fw_info->size + 6);
		r = -EINVAL;
		goto err_size;
	}

	for (i = FW_FILE_CHECKSUM_OFFSET, checksum = 0;
	     i < firmware->size; i += 2)
		checksum += firmware->data[i] + (firmware->data[i + 1] << 8);

	/* byte order change, and check */
	fw_info->checksum = le32_to_cpu(fw_info->checksum);
	if (checksum != fw_info->checksum) {
		TS_LOG_ERR("Bad firmware, cheksum error");
		r = -EINVAL;
		goto err_size;
	}

	if (fw_info->subsys_num > FW_SUBSYS_MAX_NUM) {
		TS_LOG_ERR("Bad firmware, invalid subsys num: %d",
		       fw_info->subsys_num);
		r = -EINVAL;
		goto err_size;
	}

	/* parse subsystem info */
	fw_offset = FW_HEADER_SIZE;
	for (i = 0; i < fw_info->subsys_num; i++) {
		info_offset = FW_SUBSYS_INFO_OFFSET +
					i * FW_SUBSYS_INFO_SIZE;

		fw_info->subsys[i].type = firmware->data[info_offset];
		fw_info->subsys[i].size =
		    le32_to_cpup((__le32 *)&firmware->data[info_offset + 1]);

		fw_info->subsys[i].flash_addr =
		    le32_to_cpup((__le32 *)&firmware->data[info_offset + 5]);
		if (fw_offset > firmware->size) {
			TS_LOG_ERR("Sybsys offset exceed Firmware size");
			goto err_size;
		}

		fw_info->subsys[i].data = firmware->data + fw_offset;
		fw_offset += fw_info->subsys[i].size;
	}

	TS_LOG_INFO("Firmware package protocol: V%u", fw_info->protocol_ver);
	TS_LOG_INFO("Fimware PID:GT%s", fw_info->fw_pid);
	TS_LOG_INFO("Fimware VID:%*ph", 4, fw_info->fw_vid);
	TS_LOG_INFO("Firmware chip type:%02X", fw_info->chip_type);
	TS_LOG_INFO("Firmware bus type:%s",
		(fw_info->bus_type & BUS_TYPE_SPI) ? "SPI" : "I2C");
	TS_LOG_INFO("Firmware size:%u", fw_info->size);
	TS_LOG_INFO("Firmware subsystem num:%u", fw_info->subsys_num);

	for (i = 0; i < fw_info->subsys_num; i++) {
		TS_LOG_DEBUG("------------------------------------------");
		TS_LOG_DEBUG("Index:%d", i);
		TS_LOG_DEBUG("Subsystem type:%02X", fw_info->subsys[i].type);
		TS_LOG_DEBUG("Subsystem size:%u", fw_info->subsys[i].size);
		TS_LOG_DEBUG("Subsystem flash_addr:%08X",
				fw_info->subsys[i].flash_addr);
		TS_LOG_DEBUG("Subsystem Ptr:%p", fw_info->subsys[i].data);
	}

err_size:
	return r;
}

/**
 * brl_check_update - compare the version of firmware running in
 *  touch device with the version getting from the firmware file.
 * @fw_info: firmware infomation to be compared
 * return: 0 firmware in the touch device needs to be updated
 *			< 0 no need to update firmware
 */
#define GOODIX_CFG_ID_ADDR 0x1006E
static int brl_check_update(const struct firmware_info *fw_info)
{
	struct brl_ts_version fw_ver;
	struct brl_ts_config *config;
	u32 file_cfg_id;
	u32 ic_cfg_id;
	int r;
	unsigned int res;

	config = &brl_ts->normal_cfg;
	r = brl_ts->ops.read_version(&fw_ver);

	res = memcmp(fw_ver.rom_pid, GOODIX_9897_NOCODE, NOCODE_LEN);
	if (res == 0) {
		TS_LOG_INFO("There is rom no code in the chip\n");
		goto exit;
	}
	res = memcmp(fw_ver.patch_pid, GOODIX_9897_NOCODE, NOCODE_LEN);
	if (res == 0) {
		TS_LOG_INFO("There is no code in the chip\n");
		goto exit;
	}

	if (memcmp(fw_ver.patch_pid, fw_info->fw_pid, 8)) {
		TS_LOG_ERR("Product ID is not match\n");
		return -EPERM;
	}

	/* compare config id */
	if (config->initialized) {
		file_cfg_id = brl_ts->ops.get_file_config_id(config->data, config->length);
		brl_reg_read(GOODIX_CFG_ID_ADDR, (u8 *)&ic_cfg_id, sizeof(ic_cfg_id));
		if (ic_cfg_id != file_cfg_id) {
			TS_LOG_INFO("ic_cfg_id:0x%x != file_cfg_id:0x%x\n",
				ic_cfg_id, file_cfg_id);
			goto exit;
		}
		TS_LOG_INFO("config_id is equal to the IC's\n");
	}

	res = memcmp(fw_ver.patch_vid, fw_info->fw_vid, 4);
	if (res == 0) {
		TS_LOG_ERR("FW version is equal to the IC's\n");
		return -EPERM;
	} else {
		TS_LOG_INFO("IC version[%x %x %x %x] != FW version[%x %x %x %x]\n",
			fw_ver.patch_vid[0], fw_ver.patch_vid[1], fw_ver.patch_vid[2],
			fw_ver.patch_vid[3], fw_info->fw_vid[0], fw_info->fw_vid[1],
			fw_info->fw_vid[2], fw_info->fw_vid[3]);
	}

exit:
	TS_LOG_INFO("Firmware needs to be updated\n");

	return 0;
}

/*
 * brl_load_isp - load ISP program to deivce ram
 * @fw_data: firmware data
 * return 0 ok, <0 error
 */
static int brl_load_isp(struct firmware_data *fw_data)
{
	struct brl_ts_version isp_fw_version;
	struct fw_subsys_info *fw_isp;
	int reset_gpio = brl_ts->dev_data->ts_platform_data->reset_gpio;
	u8 reg_val[8] = {0x00};
	int r;

	fw_isp = &fw_data->fw_info.subsys[0];

	TS_LOG_INFO("Loading ISP start");
	r = brl_reg_write(ISP_RAM_ADDR, (u8 *)fw_isp->data, fw_isp->size);
	if (r < 0) {
		TS_LOG_ERR("Loading ISP error");
		return r;
	}

	TS_LOG_INFO("Success send ISP data");

	/* SET BOOT OPTION TO 0X55 */
	memset(reg_val, SET_BOOT_OPTION, 8);
	r = brl_reg_write(HW_REG_CPU_RUN_FROM, reg_val, 8); /* 8:reg len */
	if (r < 0) {
		TS_LOG_ERR("Failed set REG_CPU_RUN_FROM flag");
		return r;
	}
	TS_LOG_INFO("Success write [8]0x55 to 0x%x", HW_REG_CPU_RUN_FROM);

	gpio_direction_output(reset_gpio, 0);
	usleep_range(2000, 2100); /* 2000 : 2ms */
	gpio_direction_output(reset_gpio, 1);
	msleep(100); /* 100 : 100ms */
	/* check isp state */
	r = brl_ts->ops.read_version(&isp_fw_version);
	if (r < 0) {
		TS_LOG_ERR("failed read isp version");
		return -2;
	}
	if (memcmp(&isp_fw_version.patch_pid[3], "ISP", 3)) {
		TS_LOG_ERR("patch id error %c%c%c != %s",
		isp_fw_version.patch_pid[3], isp_fw_version.patch_pid[4],
		isp_fw_version.patch_pid[5], "ISP");
		return -3;
	}

	TS_LOG_INFO("ISP running successfully");
	return 0;
}

/**
 * brl_update_prepare - update prepare, loading ISP program
 *  and make sure the ISP is running.
 * return: 0 ok, <0 error
 */
static int brl_update_prepare(struct firmware_data *fw_data)
{
	u8 reg_val[4] = {0};
	u8 temp_buf[64] = {0};
	int reset_gpio = brl_ts->dev_data->ts_platform_data->reset_gpio;
	int retry;
	int r;

	/* reset IC */
	TS_LOG_INFO("fwupdate chip reset\n");
	gpio_direction_output(reset_gpio, 0);
	usleep_range(2000, 2100); /* 2000: 2ms */
	gpio_direction_output(reset_gpio, 1);

	retry = 100;
	/* Hold cpu */
	do {
		reg_val[0] = HOLD_MCU_STAT;
		r = brl_reg_write(HOLD_CPU_REG_W, reg_val, REG_W_BUFF_LEN);
		r |= brl_reg_read(HOLD_CPU_REG_R, &temp_buf[0], 4);
		r |= brl_reg_read(HOLD_CPU_REG_R, &temp_buf[4], 4);
		r |= brl_reg_read(HOLD_CPU_REG_R, &temp_buf[8], 4);
		if (!r && !memcmp(&temp_buf[0], &temp_buf[4], 4) &&
			!memcmp(&temp_buf[4], &temp_buf[8], 4) &&
			!memcmp(&temp_buf[0], &temp_buf[8], 4)) {
			break;
		}
		usleep_range(1000, 1100); /* 1000: 1ms */
		TS_LOG_INFO("retry hold cpu %d", retry);
	} while (--retry);
	if (!retry) {
		TS_LOG_ERR("Failed to hold CPU, return =%d", r);
		return -1;
	}
	TS_LOG_INFO("Success hold CPU");

	/* enable misctl clock */
	reg_val[0] = ENABLE_MISCTL_CLOCK;
	r = brl_reg_write(MISCTL_REG, reg_val, 1);
	TS_LOG_INFO("enbale misctl clock");

	/* open ESD_KEY */
	retry = 20;
	do {
		reg_val[0] = OPEN_ESD_KEY;
		r = brl_reg_write(ESD_KEY_REG, reg_val, 1);
		r |= brl_reg_read(ESD_KEY_REG, temp_buf, 1);
		if (!r && temp_buf[0] == 0x01)
			break;
		usleep_range(1000, 1100); /* 1000 : 1ms */
		TS_LOG_INFO("retry %d enable esd key, 0x%x",
			retry, temp_buf[0]);
	} while (--retry);
	if (!retry) {
		TS_LOG_ERR("Failed to enable esd key, return =%d", r);
		return -2;
	}
	TS_LOG_INFO("success enable esd key");

	/* disable watch dog */
	reg_val[0] = DISABLE_WATCHING_DOG;
	r = brl_reg_write(WATCH_DOG_REG, reg_val, 1);
	TS_LOG_INFO("disable watch dog");

	/* load ISP code and run form isp */
	r = brl_load_isp(fw_data);
	if (r < 0)
		TS_LOG_ERR("Failed lode and run isp");

	return r;
}


#define FLASH_CMD_TYPE_READ 0xAA
#define FLASH_CMD_TYPE_WRITE 0xBB
#define FLASH_CMD_ACK_CHK_PASS 0xEE
#define FLASH_CMD_ACK_CHK_ERROR 0x33
#define FLASH_CMD_ACK_IDLE 0x11
#define FLASH_CMD_W_STATUS_CHK_PASS 0x22
#define FLASH_CMD_W_STATUS_CHK_FAIL 0x33
#define FLASH_CMD_W_STATUS_ADDR_ERR 0x44
#define FLASH_CMD_W_STATUS_WRITE_ERR 0x55
#define FLASH_CMD_W_STATUS_WRITE_OK 0xEE
/* brl_send_flash_cmd: send command to read or write flash data
 * @flash_cmd: command need to send.
 */
static int brl_send_flash_cmd(struct brl_flash_cmd *flash_cmd)
{
	int i, ret, retry;
	struct brl_flash_cmd tmp_cmd;

	TS_LOG_INFO("try send flash cmd:%*ph", (int)sizeof(flash_cmd->buf),
		flash_cmd->buf);
	memset(tmp_cmd.buf, 0, sizeof(tmp_cmd));
	ret = brl_reg_write(FLASH_CMD_REG, flash_cmd->buf,
		sizeof(flash_cmd->buf));
	if (ret) {
		TS_LOG_ERR("failed send flash cmd %d", ret);
		return ret;
	}

	retry = 5;
	for (i = 0; i < retry; i++) {
		ret = brl_reg_read(FLASH_CMD_REG,
			tmp_cmd.buf, sizeof(tmp_cmd.buf));
		if (!ret && tmp_cmd.ack == FLASH_CMD_ACK_CHK_PASS)
			break;
		usleep_range(5000, 5100); /* 5000 : 5ms */
		TS_LOG_ERR("flash cmd ack error retry %d, ack 0x%x, ret %d",
			i, tmp_cmd.ack, ret);
	}
	if (tmp_cmd.ack != FLASH_CMD_ACK_CHK_PASS) {
		TS_LOG_ERR("flash cmd ack error, ack 0x%x, ret %d",
			tmp_cmd.ack, ret);
		TS_LOG_ERR("data:%*ph",
			(int)sizeof(tmp_cmd.buf), tmp_cmd.buf);
		return -EINVAL;
	}
	TS_LOG_INFO("flash cmd ack check pass");

	msleep(100); /* 100 : 100ms */
	retry = 20;
	for (i = 0; i < retry; i++) {
		ret = brl_reg_read(FLASH_CMD_REG,
			tmp_cmd.buf, sizeof(tmp_cmd.buf));
		if (!ret && tmp_cmd.ack == FLASH_CMD_ACK_CHK_PASS &&
			tmp_cmd.status == FLASH_CMD_W_STATUS_WRITE_OK) {
			TS_LOG_INFO("flash status check pass");
			return 0;
		}

		TS_LOG_INFO("flash cmd status not ready");
		usleep_range(15000, 15100); /* 15000 : 15ms */
	}

	TS_LOG_ERR("flash cmd status error %d, ack 0x%x, status 0x%x, ret %d",
		i, tmp_cmd.ack, tmp_cmd.status, ret);
	if (ret) {
		TS_LOG_INFO("reason: bus or paltform error");
		return -EINVAL;
	}

	switch (tmp_cmd.status) {
	case FLASH_CMD_W_STATUS_CHK_PASS:
		TS_LOG_ERR("data check pass,but failed get follow-up results");
		return -EFAULT;
	case FLASH_CMD_W_STATUS_CHK_FAIL:
		TS_LOG_ERR("data check failed, please retry");
		return -EAGAIN;
	case FLASH_CMD_W_STATUS_ADDR_ERR:
		TS_LOG_ERR("flash target addr error, please check");
		return -EFAULT;
	case FLASH_CMD_W_STATUS_WRITE_ERR:
		TS_LOG_ERR("flash data write err, please retry");
		return -EAGAIN;
	default:
		TS_LOG_ERR("unknown status");
		return -EFAULT;
	}
}

#define FLASH_CMD_LEN	11
static int brl_flash_package(u8 subsys_type, u8 *pkg,
	u32 flash_addr, u16 pkg_len)
{
	int ret, retry;
	struct brl_flash_cmd flash_cmd;

	retry = 2;
	do {
		ret = brl_reg_write(HW_REG_ISP_BUFFER, pkg, pkg_len);
		if (ret < 0) {
			TS_LOG_ERR("Failed to write firmware packet");
			return ret;
		}

		flash_cmd.status = 0;
		flash_cmd.ack = 0;
		flash_cmd.len = FLASH_CMD_LEN;
		flash_cmd.cmd = FLASH_CMD_TYPE_WRITE;
		flash_cmd.fw_type = subsys_type;
		flash_cmd.fw_len = cpu_to_le16(pkg_len);
		flash_cmd.fw_addr = cpu_to_le32(flash_addr);

		goodix_append_checksum(&(flash_cmd.buf[2]),
				9, CHECKSUM_MODE_U8_LE);

		ret = brl_send_flash_cmd(&flash_cmd);
		if (!ret) {
			TS_LOG_INFO("success write package to 0x%x, len %d",
				flash_addr, pkg_len - 4);
			return 0;
		}
	} while (ret == -EAGAIN && --retry);

	return ret;
}


/**
 * brl_flash_subsystem - flash subsystem firmware,
 *  Main flow of flashing firmware.
 *	Each firmware subsystem is divided into several
 *	packets, the max size of packet is limited to
 *	@{ISP_MAX_BUFFERSIZE}
 * @subsys: subsystem infomation
 * return: 0 ok, < 0 error
 */
static int brl_flash_subsystem(struct fw_subsys_info *subsys)
{
	u16 data_size, offset;
	u32 total_size;
	u32 subsys_base_addr = subsys->flash_addr;
	u8 *fw_packet = NULL;
	int r = 0;

	/*
	 * if bus(i2c/spi) error occued, then exit, we will do
	 * hardware reset and re-prepare ISP and then retry
	 * flashing
	 */
	total_size = subsys->size;
	fw_packet = kzalloc(ISP_MAX_BUFFERSIZE + 4, GFP_KERNEL);
	if (!fw_packet) {
		TS_LOG_ERR("Failed alloc memory");
		return -EINVAL;
	}

	offset = 0;
	while (total_size > 0) {
		data_size = (total_size > ISP_MAX_BUFFERSIZE) ?
				ISP_MAX_BUFFERSIZE : total_size;
		TS_LOG_INFO("Flash firmware to %08x,size:%u bytes",
			subsys_base_addr + offset, data_size);

		memcpy(fw_packet, &subsys->data[offset], data_size);
		/* set checksum for package data */
		goodix_append_checksum(fw_packet,
				data_size, CHECKSUM_MODE_U16_LE);

		r = brl_flash_package(subsys->type, fw_packet,
				subsys_base_addr + offset, data_size + 4);
		if (r) {
			TS_LOG_ERR("failed flash to %08x,size:%u bytes",
			subsys_base_addr + offset, data_size);
			break;
		}
		offset += data_size;
		total_size -= data_size;
	} /* end while */

	kfree(fw_packet);
	return r;
}

/**
 * brl_flash_firmware - flash firmware
 * @fw_data: firmware data
 * return: 0 ok, < 0 error
 */
#define CONFIG_DATA_ADDR 0x3E000
#define CONFIG_DATA_TYPE 4
static int brl_flash_firmware(struct fw_update_ctrl *fw_ctrl)
{
	struct firmware_data *fw_data = &fw_ctrl->fw_data;
	struct  firmware_info  *fw_info = NULL;
	struct fw_subsys_info *fw_x = NULL;
	struct fw_subsys_info subsys_cfg;
	struct brl_ts_config *config = &brl_ts->normal_cfg;
	int retry = GTX8_RETRY_NUM_3;
	int i;
	int r;
	int fw_num;
	int prog_step;

	/* start from subsystem 1,subsystem 0 is the ISP program */

	fw_info = &fw_data->fw_info;
	fw_num = fw_info->subsys_num;

	/* flash config data first if we have */
	if (config->initialized) {
		memset(&subsys_cfg, 0, sizeof(subsys_cfg));
		subsys_cfg.data = config->data;
		subsys_cfg.size = config->length;
		subsys_cfg.flash_addr = CONFIG_DATA_ADDR;
		subsys_cfg.type = CONFIG_DATA_TYPE;
		r = brl_flash_subsystem(&subsys_cfg);
		if (r) {
			TS_LOG_ERR("failed flash config with ISP, %d", r);
			return r;
		}
		TS_LOG_INFO("success flash config with ISP");
	}

	/* we have 80% work here */
	prog_step = 80 / (fw_num - 1);

	for (i = 1; i < fw_num && retry;) {
		TS_LOG_INFO("--- Start to flash subsystem[%d] ---", i);
		fw_x = &fw_info->subsys[i];
		r = brl_flash_subsystem(fw_x);
		if (r == 0) {
			TS_LOG_INFO("--- End flash subsystem[%d]: OK ---", i);
			fw_ctrl->progress += prog_step;
			i++;
		} else if (r == -EAGAIN) {
			retry--;
			TS_LOG_ERR("End flash subsystem%d:Fail,errno:%d",
				i, r);
		} else if (r < 0) { /* bus error */
			TS_LOG_ERR("End flash subsystem%d:Fatal error:%d exit",
				i, r);
			goto exit_flash;
		}
	}

exit_flash:
	return r;
}

/**
 * brl_update_finish - update finished, free resource
 *  and reset flags---
 * return: 0 ok, < 0 error
 */
static int brl_update_finish(void)
{
	int ret;

	ret = brl_ts->ops.chip_reset();
	msleep(100);
	return ret;
}

int brl_fw_update_proc(void)
{
	int ret;
	int retry0 = FW_UPDATE_RETRY;
	int retry1 = FW_UPDATE_RETRY;
	ktime_t start, end;

	if (brl_update_ctrl.status == UPSTA_PREPARING ||
			brl_update_ctrl.status == UPSTA_UPDATING) {
		TS_LOG_ERR("Firmware update already in progress\n");
		return NO_ERR;
	}

	start = ktime_get();
	brl_update_ctrl.progress = FW_UPDATE_0_PERCENT;
	brl_update_ctrl.status = UPSTA_PREPARING;

	ret = brl_parse_firmware(&brl_update_ctrl.fw_data);
	if (ret < 0) {
		brl_update_ctrl.status = UPSTA_ABORT;
		goto err_parse_fw;
	}

	/*  set force update flag */
	brl_update_ctrl.progress = FW_UPDATE_10_PERCENT;
	if (brl_update_ctrl.force_update == false) {
		ret = brl_check_update(&brl_update_ctrl.fw_data.fw_info);
		if (ret < 0) {
			brl_update_ctrl.status = UPSTA_ABORT;
			goto err_check_update;
		}
	}

start_update:
	brl_update_ctrl.progress = FW_UPDATE_20_PERCENT;
	brl_update_ctrl.status = UPSTA_UPDATING; /* show upgrading status */
	do {
		ret = brl_update_prepare(&brl_update_ctrl.fw_data);
		if (ret)
			TS_LOG_ERR("failed prepare ISP, retry %d",
				FW_UPDATE_RETRY - retry0);
	} while (ret && --retry0 > 0);
	if (ret) {
		TS_LOG_ERR("Failed to prepare ISP, exit update:%d", ret);
		brl_update_ctrl.status = UPSTA_FAILED;
		goto err_fw_prepare;
	}

	/* progress: 20%~100% */
	ret = brl_flash_firmware(&brl_update_ctrl);
	if (ret == -EAGAIN && --retry1 > 0) {
		TS_LOG_ERR("Bus error, retry firmware update:%d\n",
				FW_UPDATE_RETRY - retry1);
		goto start_update;
	}
	if (ret) {
		TS_LOG_ERR("flash fw data enter error");
		brl_update_ctrl.status = UPSTA_FAILED;
		goto err_fw_flash;
	} else {
		TS_LOG_INFO("flash fw data success, need check version");
	}

	brl_update_ctrl.status = UPSTA_SUCCESS;

err_fw_flash:
err_fw_prepare:
	brl_update_finish();
err_check_update:
err_parse_fw:
	brl_ts->ops.read_version(&brl_ts->hw_info);
	if (brl_update_ctrl.status == UPSTA_SUCCESS)
		TS_LOG_INFO("Firmware update successfully\n");
	else if (brl_update_ctrl.status == UPSTA_FAILED)
		TS_LOG_ERR("Firmware update failed\n");

	brl_update_ctrl.progress = FW_UPDATE_100_PERCENT; /* 100% */
	end = ktime_get();
	brl_update_ctrl.take_time = ktime_to_ms(ktime_sub(end, start));

	msleep(20); /* 20 : 20ms */

	return ret;
}

static DEFINE_MUTEX(fwu_lock);
int brl_update_firmware(void)
{
	int ret = NO_ERR;

	mutex_lock(&fwu_lock);
	if (!brl_update_ctrl.fw_from_sysfs) {
		ret = brl_request_firmware(&brl_update_ctrl.fw_data,
				brl_update_ctrl.fw_name);
		if (ret < 0) {
			brl_update_ctrl.status = UPSTA_ABORT;
			brl_update_ctrl.progress = FW_UPDATE_100_PERCENT;
			goto out;
		}
	} else {
		if (!brl_update_ctrl.fw_data.firmware) {
			TS_LOG_ERR("Invalid firmware from sysfs\n");
			brl_update_ctrl.status = UPSTA_ABORT;
			brl_update_ctrl.progress = FW_UPDATE_100_PERCENT;
			goto out;
		}
	}

	/* ready to update */
	brl_fw_update_proc();

	/* clean */
	if (!brl_update_ctrl.fw_from_sysfs) {
		brl_release_firmware(&brl_update_ctrl.fw_data);
	} else {
		brl_update_ctrl.fw_from_sysfs = false;
		vfree(brl_update_ctrl.fw_data.firmware);
		brl_update_ctrl.fw_data.firmware = NULL;
	}

out:
	mutex_unlock(&fwu_lock);

	return ret;
}
