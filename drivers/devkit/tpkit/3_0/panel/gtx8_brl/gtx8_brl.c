#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <huawei_platform/inputhub/thphub.h>
#include "gtx8_brl.h"
#include "gtx8_brl_dts.h"
#include "huawei_ts_kit_api.h"
#include "huawei_ts_kit.h"
#if defined(CONFIG_TEE_TUI)
#include "tui.h"
#endif

#define POWER_OFF 0
#define POWER_ON 1

struct brl_ts_data *brl_ts;
static int brl_fw_update_sd(void);
static int brl_fw_update_boot(char *file_name);
static int brl_roi_switch(struct ts_roi_info *info);
static int brl_glove_switch(struct ts_glove_info *info);
static int brl_charger_switch(struct ts_charger_info *info);
static int brl_holster_switch(struct ts_holster_info *info);
static void brl_touch_switch(void);
static struct completion roi_data_done;
static u16 pre_index;
static u16 cur_index;
static u16 cur_finger_number;
static u16 pre_finger_number;

u32 brl_get_uint(u8 *buffer, int len)
{
	u32 num = 0;
	int i;

	for (i = 0; i < len; i++) {
		num <<= 8; /* 8: 8bit */
		num += buffer[i];
	}
	return num;
}

static int brl_i2c_read_transfer(u32 addr, u8 *buffer, u32 len)
{
	struct ts_kit_device_data *dev_data = brl_ts->dev_data;
	u8 data[4] = {0};
	int ret;

	data[0] = (u8)((addr >> 24) & 0xFF);
	data[1] = (u8)((addr >> 16) & 0xFF);
	data[2] = (u8)((addr >> 8) & 0xFF);
	data[3] = (u8)(addr & 0xFF);
	ret = dev_data->ts_platform_data->bops->bus_read(data,
		sizeof(data), buffer, len);
	if (ret < 0)
		TS_LOG_ERR("%s:i2c read error,addr:%04x bytes:%d\n",
			__func__, addr, len);
	return ret;
}

static int brl_i2c_write_transfer(u32 addr, u8 *buffer, u32 len)
{
	struct ts_kit_device_data *dev_data =  brl_ts->dev_data;
	u8 *data = brl_ts->write_buf;
	int ret;

	data[0] = (u8)((addr >> 24) & 0xFF);
	data[1] = (u8)((addr >> 16) & 0xFF);
	data[2] = (u8)((addr >> 8) & 0xFF);
	data[3] = (u8)(addr & 0xFF);
	memcpy(&data[4], buffer, len); /* 4: addr offset */
	ret = dev_data->ts_platform_data->bops->bus_write(data, len + 4); /* 4: addr offset */
	if (ret < 0)
		TS_LOG_ERR("%s:i2c write error,addr:%04x bytes:%d\n",
			__func__, addr, len);

	return ret;
}

int brl_i2c_read(u32 addr, u8 *buffer, u32 len)
{
	int col;
	int remain;
	int i;
	int ret = 0;
	u32 temp_addr;
	u8 *temp_buf = NULL;

	col = len / GOODIX_TRANSFER_MAX_SIZE;
	remain = len % GOODIX_TRANSFER_MAX_SIZE;
	temp_addr = addr;
	temp_buf = buffer;

	for (i = 0; i < col; i++) {
		ret = brl_i2c_read_transfer(temp_addr, temp_buf,
			GOODIX_TRANSFER_MAX_SIZE);
		if (ret)
			return ret;
		temp_addr += GOODIX_TRANSFER_MAX_SIZE;
		temp_buf += GOODIX_TRANSFER_MAX_SIZE;
	}
	if (remain > 0)
		ret = brl_i2c_read_transfer(temp_addr, temp_buf, remain);
	return ret;
}

int is_risk_data(const u8 *data, int size)
{
	int i;
	int zero_count = 0;
	int ff_count = 0;

	for (i = 0; i < size; i++) {
		if (data[i] == 0)
			zero_count++;
		else if (data[i] == 0xff)
			ff_count++;
	}
	if (zero_count == size || ff_count == size) {
		TS_LOG_ERR("warning data is all %s\n",
			zero_count == size ? "zero" : "0xff");
		return 1;
	}
	return 0;
}

int brl_i2c_write(u32 addr, u8 *buffer, u32 len)
{
	int col;
	int remain;
	int i;
	int ret = 0;
	u32 temp_addr;
	u8 *temp_buf = NULL;

	col = len / GOODIX_TRANSFER_MAX_SIZE;
	remain = len % GOODIX_TRANSFER_MAX_SIZE;
	temp_addr = addr;
	temp_buf = buffer;

	for (i = 0; i < col; i++) {
		ret = brl_i2c_write_transfer(temp_addr, temp_buf,
			GOODIX_TRANSFER_MAX_SIZE);
		if (ret)
			return ret;
		temp_addr += GOODIX_TRANSFER_MAX_SIZE;
		temp_buf += GOODIX_TRANSFER_MAX_SIZE;
	}

	if (remain > 0)
		ret = brl_i2c_write_transfer(temp_addr, temp_buf, remain);

	return ret;
}

static int convert_ic_info(struct brl_ic_info *info, const u8 *data)
{
	int i;
	struct brl_ic_info_version *version = &info->version;
	struct brl_ic_info_feature *feature = &info->feature;
	struct brl_ic_info_param *parm = &info->parm;
	struct brl_ic_info_misc *misc = &info->misc;

	info->length = le16_to_cpup((__le16 *)data);

	data += 2; /* 2: 2byte */
	memcpy(version, data, sizeof(*version));
	version->config_id = le32_to_cpu(version->config_id);

	data += sizeof(struct brl_ic_info_version);
	memcpy(feature, data, sizeof(*feature));
	feature->freqhop_feature =
		le16_to_cpu(feature->freqhop_feature);
	feature->calibration_feature =
		le16_to_cpu(feature->calibration_feature);
	feature->gesture_feature =
		le16_to_cpu(feature->gesture_feature);
	feature->side_touch_feature =
		le16_to_cpu(feature->side_touch_feature);
	feature->stylus_feature =
		le16_to_cpu(feature->stylus_feature);

	data += sizeof(struct brl_ic_info_feature);
	parm->drv_num = *(data++);
	parm->sen_num = *(data++);
	parm->button_num = *(data++);
	parm->force_num = *(data++);
	parm->active_scan_rate_num = *(data++);
	if (parm->active_scan_rate_num > MAX_SCAN_RATE_NUM) {
		TS_LOG_ERR("invalid scan rate num %d > %d\n",
			parm->active_scan_rate_num, MAX_SCAN_RATE_NUM);
		return -1;
	}
	for (i = 0; i < (parm->active_scan_rate_num); i++)
		parm->active_scan_rate[i] =
		le16_to_cpup((__le16 *)(data + i * 2));

	data += parm->active_scan_rate_num * 2;
	parm->mutual_freq_num = *(data++);
	if (parm->mutual_freq_num > MAX_SCAN_FREQ_NUM) {
		TS_LOG_ERR("invalid mntual freq num %d > %d\n",
			parm->mutual_freq_num, MAX_SCAN_FREQ_NUM);
		return -1;
	}

	for (i = 0; i < (parm->mutual_freq_num); i++)
		parm->mutual_freq[i] = le16_to_cpup((__le16 *)(data + i * 2));

	data += parm->mutual_freq_num * 2;
	parm->self_tx_freq_num = *(data++);

	if (parm->self_tx_freq_num > MAX_SCAN_FREQ_NUM) {
		TS_LOG_ERR("invalid tx freq num %d > %d\n",
			parm->self_tx_freq_num, MAX_SCAN_FREQ_NUM);
		return -1;
	}

	for (i = 0; i < (parm->self_tx_freq_num); i++)
		parm->self_tx_freq[i] = le16_to_cpup((__le16 *)(data + i * 2));

	data += parm->self_tx_freq_num * 2;
	parm->self_rx_freq_num = *(data++);
	if (parm->self_rx_freq_num > MAX_SCAN_FREQ_NUM) {
		TS_LOG_ERR("invalid rx freq num %d > %d\n",
			parm->self_rx_freq_num, MAX_SCAN_FREQ_NUM);
		return -1;
	}

	for (i = 0; i < (parm->self_rx_freq_num); i++)
		parm->self_rx_freq[i] = le16_to_cpup((__le16 *)(data + i * 2));

	data += parm->self_rx_freq_num * 2;
	parm->stylus_freq_num = *(data++);
	if (parm->stylus_freq_num > MAX_FREQ_NUM_STYLUS) {
		TS_LOG_ERR("invalid stylus freq num %d > %d\n",
			parm->stylus_freq_num, MAX_FREQ_NUM_STYLUS);
		return -1;
	}
	for (i = 0; i < (parm->stylus_freq_num); i++)
		parm->stylus_freq[i] = le16_to_cpup((__le16 *)(data + i * 2));

	data += parm->stylus_freq_num * 2;
	memcpy(misc, data, sizeof(*misc));
	misc->cmd_addr = le32_to_cpu(misc->cmd_addr);
	misc->cmd_max_len = le16_to_cpu(misc->cmd_max_len);
	misc->cmd_reply_addr = le32_to_cpu(misc->cmd_reply_addr);
	misc->cmd_reply_len = le16_to_cpu(misc->cmd_reply_len);
	misc->fw_state_addr = le32_to_cpu(misc->fw_state_addr);
	misc->fw_state_len = le16_to_cpu(misc->fw_state_len);
	misc->fw_buffer_addr = le32_to_cpu(misc->fw_buffer_addr);
	misc->fw_buffer_max_len = le16_to_cpu(misc->fw_buffer_max_len);
	misc->frame_data_addr = le32_to_cpu(misc->frame_data_addr);
	misc->frame_data_head_len = le16_to_cpu(misc->frame_data_head_len);

	misc->fw_attr_len = le16_to_cpu(misc->fw_attr_len);
	misc->fw_log_len = le16_to_cpu(misc->fw_log_len);
	misc->stylus_struct_len = le16_to_cpu(misc->stylus_struct_len);
	misc->mutual_struct_len = le16_to_cpu(misc->mutual_struct_len);
	misc->self_struct_len = le16_to_cpu(misc->self_struct_len);
	misc->noise_struct_len = le16_to_cpu(misc->noise_struct_len);
	misc->touch_data_addr = le32_to_cpu(misc->touch_data_addr);
	misc->touch_data_head_len = le16_to_cpu(misc->touch_data_head_len);
	misc->point_struct_len = le16_to_cpu(misc->point_struct_len);
	misc->mutual_rawdata_addr = le32_to_cpu(misc->mutual_rawdata_addr);
	misc->mutual_diffdata_addr = le32_to_cpu(misc->mutual_diffdata_addr);
	misc->mutual_refdata_addr = le32_to_cpu(misc->mutual_refdata_addr);
	misc->self_rawdata_addr = le32_to_cpu(misc->self_rawdata_addr);
	misc->self_diffdata_addr = le32_to_cpu(misc->self_diffdata_addr);
	misc->self_refdata_addr = le32_to_cpu(misc->self_refdata_addr);
	misc->iq_rawdata_addr = le32_to_cpu(misc->iq_rawdata_addr);
	misc->iq_refdata_addr = le32_to_cpu(misc->iq_refdata_addr);
	misc->im_rawdata_addr = le32_to_cpu(misc->im_rawdata_addr);
	misc->im_readata_len = le16_to_cpu(misc->im_readata_len);
	misc->noise_rawdata_addr = le32_to_cpu(misc->noise_rawdata_addr);
	misc->noise_rawdata_len = le16_to_cpu(misc->noise_rawdata_len);
	misc->stylus_rawdata_addr = le32_to_cpu(misc->stylus_rawdata_addr);
	misc->stylus_rawdata_len = le16_to_cpu(misc->stylus_rawdata_len);
	misc->noise_data_addr = le32_to_cpu(misc->noise_data_addr);
	misc->esd_addr = le32_to_cpu(misc->esd_addr);

	return 0;
}

static void print_ic_info(struct brl_ic_info *ic_info)
{
	struct brl_ic_info_version *version = &ic_info->version;
	struct brl_ic_info_feature *feature = &ic_info->feature;
	struct brl_ic_info_param *parm = &ic_info->parm;
	struct brl_ic_info_misc *misc = &ic_info->misc;

	TS_LOG_INFO("ic_info_length:                %d",
		ic_info->length);
	TS_LOG_INFO("info_customer_id:              0x%01X",
		version->info_customer_id);
	TS_LOG_INFO("info_version_id:               0x%01X",
		version->info_version_id);
	TS_LOG_INFO("ic_die_id:                     0x%01X",
		version->ic_die_id);
	TS_LOG_INFO("ic_version_id:                 0x%01X",
		version->ic_version_id);
	TS_LOG_INFO("config_id:                     0x%4X",
		version->config_id);
	TS_LOG_INFO("config_version:                0x%01X",
		version->config_version);
	TS_LOG_INFO("frame_data_customer_id:        0x%01X",
		version->frame_data_customer_id);
	TS_LOG_INFO("frame_data_version_id:         0x%01X",
		version->frame_data_version_id);
	TS_LOG_INFO("touch_data_customer_id:        0x%01X",
		version->touch_data_customer_id);
	TS_LOG_INFO("touch_data_version_id:         0x%01X",
		version->touch_data_version_id);

	TS_LOG_INFO("freqhop_feature:               0x%04X",
		feature->freqhop_feature);
	TS_LOG_INFO("calibration_feature:           0x%04X",
		feature->calibration_feature);
	TS_LOG_INFO("gesture_feature:               0x%04X",
		feature->gesture_feature);
	TS_LOG_INFO("side_touch_feature:            0x%04X",
		feature->side_touch_feature);
	TS_LOG_INFO("stylus_feature:                0x%04X",
		feature->stylus_feature);

	TS_LOG_INFO("Drv*Sen,Button,Force num:      %d x %d, %d, %d",
		parm->drv_num, parm->sen_num,
		parm->button_num, parm->force_num);

	TS_LOG_INFO("Cmd:                           0x%04X, %d",
		misc->cmd_addr, misc->cmd_max_len);
	TS_LOG_INFO("Cmd-Reply:                     0x%04X, %d",
		misc->cmd_reply_addr, misc->cmd_reply_len);
	TS_LOG_INFO("FW-State:                      0x%04X, %d",
		misc->fw_state_addr, misc->fw_state_len);
	TS_LOG_INFO("FW-Buffer:                     0x%04X, %d",
		misc->fw_buffer_addr, misc->fw_buffer_max_len);
	TS_LOG_INFO("Touch-Data:                    0x%04X, %d",
		misc->touch_data_addr, misc->touch_data_head_len);
	TS_LOG_INFO("point_struct_len:              %d",
		misc->point_struct_len);
	TS_LOG_INFO("mutual_rawdata_addr:           0x%04X",
		misc->mutual_rawdata_addr);
	TS_LOG_INFO("mutual_diffdata_addr:          0x%04X",
		misc->mutual_diffdata_addr);
	TS_LOG_INFO("self_rawdata_addr:             0x%04X",
		misc->self_rawdata_addr);
	TS_LOG_INFO("self_rawdata_addr:             0x%04X",
		misc->self_rawdata_addr);
	TS_LOG_INFO("stylus_rawdata_addr:           0x%04X, %d",
		misc->stylus_rawdata_addr, misc->stylus_rawdata_len);
	TS_LOG_INFO("esd_addr:                      0x%04X",
		misc->esd_addr);
}

#define GOODIX_GET_IC_INFO_RETRY 3
#define GOODIX_IC_INFO_MAX_LEN 1024
#define GOODIX_IC_INFO_ADDR 0x10068
static int brl_get_ic_info(struct brl_ic_info *ic_info)
{
	int ret = 0;
	int i;
	u16 length = 0;
	u8 afe_data[GOODIX_IC_INFO_MAX_LEN] = {0};
	u8 checksum_err_flag;

	for (i = 0; i < GOODIX_GET_IC_INFO_RETRY; i++) {
		checksum_err_flag = 0;
		ret = brl_i2c_read(GOODIX_IC_INFO_ADDR,
			(u8 *)&length, sizeof(length));
		if (ret < 0) {
			TS_LOG_ERR("failed get ic info lenght\n");
			usleep_range(5000, 5100); /* 5000: 5ms */
			continue;
		}

		length = le16_to_cpu(length);
		if (length >= GOODIX_IC_INFO_MAX_LEN) {
			TS_LOG_ERR("invalid ic info length %d, retry %d\n",
				length, i);
			continue;
		}
		ret = brl_i2c_read(GOODIX_IC_INFO_ADDR, afe_data, length);
		if (ret < 0) {
			TS_LOG_ERR("failed get ic info data\n");
			usleep_range(5000, 5100); /* 5000: 5ms */
			continue;
		}
		if (is_risk_data((const uint8_t *)afe_data, length)) {
			TS_LOG_ERR("fw info data invalid\n");
			usleep_range(5000, 5100);
			continue;
		}
		if (checksum_cmp((const uint8_t *)afe_data,
			length, CHECKSUM_MODE_U8_LE)) {
			TS_LOG_ERR("fw info checksum error\n!");
			usleep_range(5000, 5100);
			checksum_err_flag = 1;
			continue;
		}
		break;
	}

	if (i == GOODIX_GET_IC_INFO_RETRY && !checksum_err_flag) {
		TS_LOG_ERR("failed get ic info\n");
		return -1;
	}

	ret = convert_ic_info(ic_info, afe_data);
	if (ret) {
		TS_LOG_ERR("convert ic info encounter error\n");
		return ret;
	}
	print_ic_info(ic_info);

	/* check some key info */
	if (!ic_info->misc.cmd_addr || !ic_info->misc.fw_buffer_addr ||
		!ic_info->misc.touch_data_addr) {
		TS_LOG_ERR("cmd_addr fw_buf_addr and touch_data_addr is null\n");
		return -1;
	}
	return 0;
}


#define GOODIX_PROJECT_ID_ADDR 0x10028
#define GOODIX_PROJECT_ID_LEN 10
static brl_get_project_id(struct brl_ts_data *ts)
{
	u8 buf[GOODIX_PROJECT_ID_LEN + 1] = {0};
	int ret;

	ret = brl_i2c_read(GOODIX_PROJECT_ID_ADDR, buf, GOODIX_PROJECT_ID_LEN);
	if (ret < 0)
		return ret;
	strncpy(ts->project_id, buf, MAX_STR_LEN);
	TS_LOG_INFO("project_id:%s\n", ts->project_id);
	return ret;
}


u32 goodix_append_checksum(u8 *data, int len, int mode)
{
	u32 checksum;
	int i;

	checksum = 0;
	if (mode == CHECKSUM_MODE_U8_LE) {
		for (i = 0; i < len; i++)
			checksum += data[i];
	} else {
		for (i = 0; i < len; i += 2)
			checksum += (data[i] + (data[i + 1] << 8));
	}

	if (mode == CHECKSUM_MODE_U8_LE) {
		data[len] = checksum & 0xff;
		data[len + 1] = (checksum >> 8) & 0xff;
		return 0xFFFF & checksum;
	}

	data[len] = checksum & 0xff;
	data[len + 1] = (checksum >> 8) & 0xff;
	data[len + 2] = (checksum >> 16) & 0xff;
	data[len + 3] = (checksum >> 24) & 0xff;

	return checksum;
}

/*
 * checksum_cmp: check data valid or not
 * @data: data need to be check
 * @size: data length need to be check(include the checksum bytes)
 * @mode: compare with U8 or U16 mode
 */
int checksum_cmp(const u8 *data, int size, int mode)
{
	u32 cal_checksum = 0;
	u32 r_checksum;
	int i;

	if (mode == CHECKSUM_MODE_U8_LE) {
		if (size < 2)
			return 1;
		for (i = 0; i < size - 2; i++)
			cal_checksum += data[i];

		r_checksum = data[size - 2] + (data[size - 1] << 8);

		if (r_checksum != cal_checksum)
			TS_LOG_ERR("CHECKSUM_MODE_U8_LE cal_checksum:%x, r_checksum:%x\n",
				cal_checksum, r_checksum);
		return (cal_checksum & 0xFFFF) == r_checksum ? 0 : 1;
	}

	if (size < 4)
		return 1;
	for (i = 0; i < size - 4; i += 2)
		cal_checksum += data[i] + (data[i + 1] << 8);
	r_checksum = data[size - 4] + (data[size - 3] << 8) +
		(data[size - 2] << 16) + (data[size - 1] << 24);

	if (r_checksum != cal_checksum)
		TS_LOG_ERR("u16 cal_checksum:%x, r_checksum:%x\n",
			cal_checksum, r_checksum);
	return (cal_checksum == r_checksum) ? 0 : 1;
}

#define CMD_ACK_OK 0x80
static DEFINE_MUTEX(cmd_mutex);
static int brl_send_cmd(struct brl_ts_cmd *cmd)
{
	int ret;
	int retry;
	int try_total = GTX8_RETRY_NUM_3;
	u8 ack_buf[2] = {0}; /* 2:ack buf size */
	u32 reg = brl_ts->ic_info.misc.cmd_addr;

	cmd->state = 0;
	cmd->ack = 0;
	goodix_append_checksum(&(cmd->buf[2]), cmd->len - 2,
		CHECKSUM_MODE_U8_LE);
	if (cmd->len == 0 || cmd->len > MAX_CMD_BUF_LEN - 2) {
		TS_LOG_ERR("cmd len error\n");
		return -EINVAL;
	}

	mutex_lock(&cmd_mutex);
	while (try_total--) {
		ret = brl_i2c_write(reg, cmd->buf, cmd->len + CHECKSUM_LEN);
		if (ret < 0) {
			TS_LOG_ERR("write cmd buf failed, ret:%d\n", ret);
			usleep_range(USLEEP_START, USLEEP_END);
			continue;
		}
		retry = 5; /* 5:retry 5 times */
		while (retry--) {
			usleep_range(USLEEP_START, USLEEP_END);
			ret = brl_i2c_read(reg, ack_buf, sizeof(ack_buf));
			if (!ret && ack_buf[1] == CMD_ACK_OK)
				break;
		}
		if (retry >= 0)
			break;
		TS_LOG_INFO("Can't read back valid ack, ret:%d ack:%02x, retry:%d\n", ret, ack_buf[1], try_total);
		msleep(20); /* 20:retry delay */
	}

	if (try_total < 0) {
		TS_LOG_ERR("Failed to send command\n");
		ret = -EINVAL;
		goto exit;
	}

	msleep(20); /* 20:wait for command work 20ms */

exit:
	mutex_unlock(&cmd_mutex);
	return ret;
}

static int wait_cmd_status(u8 target_status, int retry)
{
	struct brl_ts_cmd cmd_ack;
	u32 reg = brl_ts->ic_info.misc.cmd_addr;
	int i;
	int ret;

	for (i = 0; i < retry; i++) {
		ret = brl_i2c_read(reg, cmd_ack.buf, sizeof(cmd_ack));
		if (!ret && cmd_ack.state == target_status) {
			TS_LOG_INFO("status check pass\n");
			return 0;
		}
		TS_LOG_INFO("cmd buf %*ph\n", (int)sizeof(cmd_ack), cmd_ack.buf);
		msleep(20);
	}
	TS_LOG_ERR("cmd status not ready, retry %d, ack 0x%02x, status 0x%02x, ret %d\n",
			i, cmd_ack.ack, cmd_ack.state, ret);
	return -EINVAL;
}

#define CONFIG_CMD_STATUS_PASS 0x80
#define CONFIG_CMD_WAIT_RETRY 20
static int send_cfg_cmd(struct brl_ts_cmd *cmd)
{
	int ret;

	ret = brl_send_cmd(cmd);
	if (ret) {
		TS_LOG_ERR("failed write cfg prepare cmd %d\n", ret);
		return ret;
	}
	ret = wait_cmd_status(CONFIG_CMD_STATUS_PASS, CONFIG_CMD_WAIT_RETRY);
	if (ret) {
		TS_LOG_ERR("failed wait for fw ready for config, %d\n", ret);
		return ret;
	}
	return 0;
}

static int brl_send_config(struct brl_ts_config *config)
{
	int ret;
	u8 *tmp_buf = NULL;
	int retry = GTX8_RETRY_NUM_3;
	struct brl_ts_cmd cfg_cmd;
	u32 max_len = brl_ts->ic_info.misc.fw_buffer_max_len;
	u32 reg_cfg = brl_ts->ic_info.misc.fw_buffer_addr;

	if (config->length > max_len) {
		TS_LOG_ERR("config len exceed limit %d > %d\n",
			config->length, max_len);
		return -EINVAL;
	}

	tmp_buf = kzalloc(config->length, GFP_KERNEL);
	if (!tmp_buf) {
		TS_LOG_ERR("failed alloc malloc\n");
		return -ENOMEM;
	}

	while (retry--) {
		memset(cfg_cmd.buf, 0, sizeof(cfg_cmd));
		cfg_cmd.len = CONFIG_CMD_LEN;
		cfg_cmd.cmd = CONFIG_CMD_START;
		ret = send_cfg_cmd(&cfg_cmd);
		if (ret) {
			TS_LOG_ERR("failed write cfg prepare cmd %d\n", ret);
			msleep(20); /* 20:wait delay */
			continue;
		}
		ret = brl_i2c_write(reg_cfg, config->data, config->length);
		if (ret) {
			TS_LOG_ERR("failed write config data, %d\n", ret);
			goto _retry;
		}
		ret = brl_i2c_read(reg_cfg, tmp_buf, config->length);
		if (ret) {
			TS_LOG_ERR("failed read back config data\n");
			goto _retry;
		}
		if (memcmp(config->data, tmp_buf, config->length)) {
			TS_LOG_ERR("config data read back compare file\n");
			TS_LOG_INFO("write_buf:%*ph\n", config->length, config->data);
			TS_LOG_INFO("read_buf:%*ph\n", config->length, tmp_buf);
			ret = -EINVAL;
			goto _retry;
		}
		/* notify fw for recive config */
		memset(cfg_cmd.buf, 0, sizeof(cfg_cmd));
		cfg_cmd.len = CONFIG_CMD_LEN;
		cfg_cmd.cmd = CONFIG_CMD_WRITE;
		ret = send_cfg_cmd(&cfg_cmd);
		if (ret)
			TS_LOG_ERR("failed send config data ready cmd %d\n", ret);

_retry:
		memset(cfg_cmd.buf, 0, sizeof(cfg_cmd));
		cfg_cmd.len = CONFIG_CMD_LEN;
		cfg_cmd.cmd = CONFIG_CMD_EXIT;
		send_cfg_cmd(&cfg_cmd);
		if (ret) {
			TS_LOG_INFO("send config process error, retry:%d\n", retry);
			msleep(20); /* 20:wait delay */
			continue;
		}
		break;
	}
	if (retry < 0)
		TS_LOG_ERR("send config failed, ret:%d\n", ret);
	else
		TS_LOG_INFO("send config success\n");

	kfree(tmp_buf);
	return ret;
}

/* get config_id from config file */
#define CONFIG_ID_OFFSET 30
static u32 brl_get_file_config_id(u8 *data, u32 len)
{
	if (!data || !len)
		return 0;
	return le32_to_cpup((__le32 *)&data[CONFIG_ID_OFFSET]);
}

/*
 * return: return config length on succes, other wise return < 0
 */
static int brl_read_config(u8 *cfg, u32 size)
{
	int ret;
	int retry = GTX8_RETRY_NUM_3;
	struct brl_ts_cmd cfg_cmd;
	struct brl_config_head cfg_head;
	u32 max_len = brl_ts->ic_info.misc.fw_buffer_max_len;
	u32 reg_cfg = brl_ts->ic_info.misc.fw_buffer_addr;

	if (!cfg)
		return -EINVAL;

	while (retry--) {
		memset(cfg_cmd.buf, 0, sizeof(cfg_cmd));
		cfg_cmd.len = CONFIG_CMD_LEN;
		cfg_cmd.cmd = CONFIG_CMD_READ_START;
		ret = send_cfg_cmd(&cfg_cmd);
		if (ret) {
			TS_LOG_ERR("failed send config read prepare command\n");
			msleep(20); /* 20:wait delay */
			continue;
		}
		ret = brl_i2c_read(reg_cfg, cfg_head.buf, sizeof(cfg_head));
		if (ret) {
			TS_LOG_ERR("failed read config head %d\n", ret);
			goto _retry;
		}
		if (checksum_cmp(cfg_head.buf, sizeof(cfg_head), CHECKSUM_MODE_U8_LE)) {
			TS_LOG_ERR("config head checksum error\n");
			TS_LOG_INFO("cfg_head:%*ph\n", sizeof(cfg_head), cfg_head.buf);
			ret = -EINVAL;
			goto _retry;
		}
		cfg_head.cfg_len = le16_to_cpu(cfg_head.cfg_len);
		if (cfg_head.cfg_len > max_len ||
			cfg_head.cfg_len > size) {
			TS_LOG_ERR("cfg len exceed buffer size %d > %d\n",
				cfg_head.cfg_len, max_len);
			ret = -EINVAL;
			goto _retry;
		}
		memcpy(cfg, cfg_head.buf, sizeof(cfg_head));
		ret = brl_i2c_read(reg_cfg + sizeof(cfg_head),
			cfg + sizeof(cfg_head), cfg_head.cfg_len);
		if (ret) {
			TS_LOG_ERR("failed read cfg pack, %d\n", ret);
			goto _retry;
		}
		TS_LOG_INFO("config len %d\n", cfg_head.cfg_len);
		if (checksum_cmp(cfg + sizeof(cfg_head),
			cfg_head.cfg_len, CHECKSUM_MODE_U16_LE)) {
			TS_LOG_ERR("config body checksum error\n");
			TS_LOG_ERR("config_body:%*ph\n", cfg_head.cfg_len, cfg + sizeof(cfg_head));
			ret = -EINVAL;
		}

_retry:
		memset(cfg_cmd.buf, 0, sizeof(cfg_cmd));
		cfg_cmd.len = CONFIG_CMD_LEN;
		cfg_cmd.cmd = CONFIG_CMD_READ_EXIT;
		send_cfg_cmd(&cfg_cmd);
		if (ret) {
			TS_LOG_INFO("read config process error, retry:%d\n", retry);
			msleep(20); /* 20:wait delay */
			continue;
		}
		break;
	}

	if (retry < 0) {
		TS_LOG_ERR("read config failed, ret:%d\n", ret);
		return -EINVAL;
	}
	TS_LOG_INFO("success read config data: len %zu\n",
		cfg_head.cfg_len + sizeof(cfg_head));
	return cfg_head.cfg_len + sizeof(cfg_head);
}

static int brl_read_version(struct brl_ts_version *version)
{
	u32 len = sizeof(struct brl_ts_version);
	int ret;

	ret = brl_i2c_read(FW_VERSION_INFO_ADDR, (u8 *)version, len);
	if (ret < 0)
		TS_LOG_ERR("%s:read version failed!", __func__);

	TS_LOG_INFO("rom_pid:%s", version->rom_pid);
	TS_LOG_INFO("rom_vid:%02x %02x %02x",
		version->rom_vid[0], version->rom_vid[1],
		version->rom_vid[2]);
	TS_LOG_INFO("patch_pid:%s", version->patch_pid);
	TS_LOG_INFO("patch_vid:%02x %02x %02x %02x",
		version->patch_vid[0], version->patch_vid[1],
		version->patch_vid[2], version->patch_vid[3]);
	TS_LOG_INFO("sensor_id:%d", version->sensor_id);

	snprintf(brl_ts->dev_data->version_name, MAX_STR_LEN - 1,
		"0x%04X", get_u32(version->patch_pid));

	return ret;
}

static int brl_prepare_parse_dts(struct ts_kit_platform_data *pdata)
{
	int ret;
	u32 value = 0;

	ret = of_property_read_u32(pdata->chip_data->cnode,
			GTX8_SLAVE_ADDR, &value);
	if (ret) {
		TS_LOG_ERR(
			"%s:get device slave_address failed, use default value",
			__func__);
		value = GTX8_DEFAULT_SLAVE_ADDR;
	}
	pdata->client->addr = value;
	TS_LOG_INFO("%s i2c addr:0x%x\n", __func__, pdata->client->addr);
	return NO_ERR;
}

/*
 * brl_pinctrl_init - pinctrl init
 */
static int brl_pinctrl_init(void)
{
	int ret = NO_ERR;
	struct brl_ts_data *ts = brl_ts;

	ts->pinctrl = devm_pinctrl_get(&ts->pdev->dev);
	if (IS_ERR_OR_NULL(ts->pinctrl)) {
		TS_LOG_INFO("%s : Failed to get pinctrl[ignore]:%ld",
			__func__, PTR_ERR(ts->pinctrl));
		ts->pinctrl = NULL;
		/* Ignore this error to support no pinctrl platform */
		return NO_ERR;
	}

	ts->pins_default = pinctrl_lookup_state(ts->pinctrl, "default");
	if (IS_ERR_OR_NULL(ts->pins_default)) {
		TS_LOG_ERR("%s: Pin state[default] not found", __func__);
		ret = PTR_ERR(ts->pins_default);
		goto exit_put;
	}

	ts->pins_suspend = pinctrl_lookup_state(ts->pinctrl, "idle");
	if (IS_ERR_OR_NULL(ts->pins_suspend)) {
		TS_LOG_ERR("%s: Pin state[idle] not found[ignore]:%ld",
			__func__, PTR_ERR(ts->pins_suspend));
		ts->pins_suspend = NULL;
		/* permit undefine idle state */
		ret = NO_ERR;
	}

	return ret;
exit_put:
	devm_pinctrl_put(ts->pinctrl);
	ts->pinctrl = NULL;
	ts->pins_suspend = NULL;
	ts->pins_default = NULL;

	return ret;
}

static void brl_pinctrl_release(void)
{
	struct brl_ts_data *ts = brl_ts;

	if (ts->pinctrl)
		devm_pinctrl_put(ts->pinctrl);
	ts->pinctrl = NULL;
	ts->pins_gesture = NULL;
	ts->pins_suspend = NULL;
	ts->pins_default = NULL;
}

/**
 * brl_chip_reset - reset chip
 */
static int brl_chip_reset(void)
{
	int reset_gpio = brl_ts->dev_data->ts_platform_data->reset_gpio;

	TS_LOG_INFO("Chip reset");
	gpio_direction_output(reset_gpio, 0);
	udelay(2000); /* 2000: 2ms */
	gpio_direction_output(reset_gpio, 1);
	msleep(100); /* 100: 100ms */
	return 0;
}

/**
 * brl_pinctrl_select_normal - set normal pin state
 *  Irq pin *must* be set to *pull-up* state.
 */
static void brl_pinctrl_select_normal(void)
{
	int ret;
	struct brl_ts_data *ts = brl_ts;

	if (ts->pinctrl && ts->pins_default) {
		ret = pinctrl_select_state(ts->pinctrl, ts->pins_default);
		if (ret < 0)
			TS_LOG_ERR("%s:Set normal pin state error:%d",
				__func__, ret);
	}
}

/**
 * brl_pinctrl_select_suspend - set suspend pin state
 *  Irq pin *must* be set to *pull-up* state.
 */
static void brl_pinctrl_select_suspend(void)
{
	int ret;
	struct brl_ts_data *ts = brl_ts;

	if (ts->pinctrl && ts->pins_suspend) {
		ret = pinctrl_select_state(ts->pinctrl, ts->pins_suspend);
		if (ret < 0)
			TS_LOG_ERR("%s:Set suspend pin state error:%d",
				__func__, ret);
	}
}

#define GOODIX_REG_CLK_STA0 0xD807
#define GOODIX_CLK_STA0_ENABLE 0xFF
#define GOODIX_REG_CLK_STA1 0xD806
#define GOODIX_CLK_STA1_ENABLE 0x77
#define GOODIX_REG_TRIM_D12 0xD006
#define GOODIX_TRIM_D12_LEVEL 0x3C
#define GOODIX_REG_RESET 0xD808
#define GOODIX_RESET_EN 0xFA
#define HOLD_CPU_REG_R 0x2000
#define BUF_LEN 4
#define REG_VAL_LEN 2
#define TEMP_BUF_LEN 12
static int brl_reset_later(void)
{
	u8 reg_val[REG_VAL_LEN] = {0};
	u8 temp_buf[TEMP_BUF_LEN] = {0};
	int ret = 0;
	int retry;

	TS_LOG_INFO("reset later in\n");
	usleep_range(USLEEP_START, USLEEP_END);

	/* hold cpu */
	retry = GTX8_RETRY_NUM_10;
	while (retry--) {
		reg_val[0] = HOLD_MCU_STAT;
		ret = brl_i2c_write(HOLD_CPU_REG_W, reg_val, REG_W_BUFF_LEN);
		if (ret) {
			TS_LOG_ERR("failed write , %d\n", ret);
			return ret;
		}
		ret = brl_i2c_read(HOLD_CPU_REG_R, &temp_buf[0], BUF_LEN);
		if (ret) {
			TS_LOG_ERR("failed read , %d\n", ret);
			return ret;
		}
		ret = brl_i2c_read(HOLD_CPU_REG_R, &temp_buf[4], BUF_LEN);
		if (ret) {
			TS_LOG_ERR("failed read , %d\n", ret);
			return ret;
		}
		ret = brl_i2c_read(HOLD_CPU_REG_R, &temp_buf[8], BUF_LEN);
		if (ret) {
			TS_LOG_ERR("failed read , %d\n", ret);
			return ret;
		}
		if (!memcmp(&temp_buf[0], &temp_buf[4], BUF_LEN) &&
			!memcmp(&temp_buf[4], &temp_buf[8], BUF_LEN) &&
			!memcmp(&temp_buf[0], &temp_buf[8], BUF_LEN))
			break;
	}
	if (retry < 0) {
		TS_LOG_ERR("Failed to hold cpu\n");
		return -EINVAL;
	}

	/* enable sta0 clk */
	retry = GTX8_RETRY_NUM_3;
	while (retry--) {
		reg_val[0] = GOODIX_CLK_STA0_ENABLE;
		ret = brl_i2c_write(GOODIX_REG_CLK_STA0, reg_val, 1);
		if (ret) {
			TS_LOG_ERR("failed write , %d\n", ret);
			return ret;
		}
		ret = brl_i2c_read(GOODIX_REG_CLK_STA0, temp_buf, 1);
		if (ret) {
			TS_LOG_ERR("failed read , %d\n", ret);
			return ret;
		}
		if (temp_buf[0] == reg_val[0])
			break;
	}
	if (retry < 0) {
		TS_LOG_ERR("Failed to enable group0 clock\n");
		return -EINVAL;
	}

	/* enable sta1 clk */
	retry = GTX8_RETRY_NUM_3;
	while (retry--) {
		reg_val[0] = GOODIX_CLK_STA1_ENABLE;
		ret = brl_i2c_write(GOODIX_REG_CLK_STA1, reg_val, 1);
		if (ret) {
			TS_LOG_ERR("failed write , %d\n", ret);
			return ret;
		}
		ret = brl_i2c_read(GOODIX_REG_CLK_STA1, temp_buf, 1);
		if (ret) {
			TS_LOG_ERR("failed read , %d\n", ret);
			return ret;
		}
		if (temp_buf[0] == reg_val[0])
			break;
	}
	if (retry < 0) {
		TS_LOG_ERR("Failed to enable group1 clock\n");
		return -EINVAL;
	}

	/* set D12 level */
	retry = GTX8_RETRY_NUM_3;
	while (retry--) {
		reg_val[0] = GOODIX_TRIM_D12_LEVEL;
		ret = brl_i2c_write(GOODIX_REG_TRIM_D12, reg_val, 1);
		if (ret) {
			TS_LOG_ERR("failed write , %d\n", ret);
			return ret;
		}
		ret = brl_i2c_read(GOODIX_REG_TRIM_D12, temp_buf, 1);
		if (ret) {
			TS_LOG_ERR("failed read , %d\n", ret);
			return ret;
		}
		if (temp_buf[0] == reg_val[0])
			break;
	}
	if (retry < 0) {
		TS_LOG_ERR("Failed to set D12\n");
		return -EINVAL;
	}

	usleep_range(USLEEP_START, USLEEP_END);
	/* soft reset */
	reg_val[0] = GOODIX_RESET_EN;
	ret = brl_i2c_write(GOODIX_REG_RESET, reg_val, 1);
	if (ret < 0)
		return ret;

	TS_LOG_INFO("reset later out\n");
	return 0;
}

static void brl_power_on_gpio_set(void)
{
	struct brl_ts_data *ts = brl_ts;

	brl_pinctrl_select_normal();
	gpio_direction_input(ts->dev_data->ts_platform_data->irq_gpio);
	gpio_direction_output(ts->dev_data->ts_platform_data->reset_gpio, 1);
}

static void brl_power_off_gpio_set(void)
{
	struct brl_ts_data *ts = brl_ts;

	brl_pinctrl_select_suspend();
	gpio_direction_input(ts->dev_data->ts_platform_data->irq_gpio);
	gpio_direction_output(ts->dev_data->ts_platform_data->reset_gpio, 0);
}

/*
 * brl_power_switch - power switch
 * @on: 1-switch on, 0-switch off
 * return: 0-succeed, -1-faileds
 */
static int brl_power_init(void)
{
	int ret;

	ret = ts_kit_power_supply_get(TS_KIT_IOVDD);
	if (ret)
		return ret;
	ret = ts_kit_power_supply_get(TS_KIT_VCC);

	return ret;
}

static int brl_power_release(void)
{
	int ret;

	ret = ts_kit_power_supply_put(TS_KIT_IOVDD);
	if (ret)
		return ret;
	ret = ts_kit_power_supply_put(TS_KIT_VCC);

	return ret;
}

static int brl_power_on(void)
{
	int ret;

	TS_LOG_INFO("%s: called", __func__);

	ret = ts_kit_power_supply_ctrl(TS_KIT_IOVDD, TS_KIT_POWER_ON, 2);
	if (ret)
		return ret;
	ret = ts_kit_power_supply_ctrl(TS_KIT_VCC, TS_KIT_POWER_ON, 15); /* 15: 15ms */
	if (ret)
		return ret;

	brl_power_on_gpio_set();
	ret = brl_reset_later();
	return ret;
}

static int  brl_power_off(void)
{
	int ret;

	ret = ts_kit_power_supply_ctrl(TS_KIT_IOVDD, TS_KIT_POWER_OFF, 0);
	if (ret)
		return ret;
	ret = ts_kit_power_supply_ctrl(TS_KIT_VCC, TS_KIT_POWER_OFF, 0);
	if (ret)
		return ret;
	udelay(2);

	brl_power_off_gpio_set();
	return 0;
}

/*
 * brl_power_switch - power switch
 * @on: 1-switch on, 0-switch off
 * return: 0-succeed, -1-faileds
 */
static int brl_power_switch(int on)
{
	int reset_gpio = 0;
	int ret;
	struct brl_ts_data *ts = brl_ts;

	reset_gpio = ts->dev_data->ts_platform_data->reset_gpio;
	ret = gpio_direction_output(reset_gpio, 0);
	if (ret)
		TS_LOG_ERR("%s:error./", __func__);
	udelay(100); /* 100: 100us */
	if (on)
		ret = brl_power_on();
	else
		ret = brl_power_off();
	return 0;
}

/*
 * brl_switch_workmode - Switch working mode.
 * @workmode: GTP_CMD_SLEEP - Sleep mode
 *			  GESTURE_MODE - gesture mode
 * Returns  0--success,non-0--fail.
 */
static int brl_switch_workmode(int workmode)
{
	s32 retry = 0;
	struct brl_ts_cmd cmd;

	switch (workmode) {
	case SLEEP_MODE:
		cmd.cmd = GOODIX_SLEEP_CMD;
		cmd.len = CONFIG_CMD_LEN;
		break;
	case GESTURE_MODE:
		cmd.cmd = GOODIX_GESTURE_CMD;
		cmd.len = CONFIG_CMD_LEN;
		break;
	default:
		TS_LOG_ERR("%s: no supported workmode", __func__);
		return -EINVAL;
	}
	while (retry++ < GTX8_RETRY_NUM_3) {
		if (!brl_send_cmd(&cmd))
			return NO_ERR;
		TS_LOG_ERR("%s: send_cmd failed, retry = %d", __func__, retry);
	}
	TS_LOG_ERR("%s: Failed to switch working mode", __func__);
	return -EINVAL;
}


/*
 * brl_feature_resume - firmware feature resume
 * @ts: pointer to brl_ts_data
 * return 0 ok, others error
 */
static int brl_feature_resume(struct brl_ts_data *ts)
{
	struct brl_ts_config *config = NULL;
	u32 config_id;

	config = &ts->normal_cfg;

	if (!config->initialized) {
		TS_LOG_INFO("no valid normal config found\n");
		return NO_ERR;
	}

	config_id = brl_get_file_config_id(config->data, config->length);
	if (ts->ic_info.version.config_id == config_id) {
		TS_LOG_INFO("config id is equal 0x%x, skiped\n", config_id);
		return NO_ERR;
	}

	TS_LOG_INFO("try send config, id=0x%x\n", config_id);
	return brl_send_config(config);
}

static int brl_status_resume(void)
{
	int ret = NO_ERR;
	struct ts_feature_info *info =
		&brl_ts->dev_data->ts_platform_data->feature_info;
	struct ts_roi_info roi_info = info->roi_info;
	struct ts_glove_info glove_info = info->glove_info;
	struct ts_charger_info charger_info = info->charger_info;

	if (roi_info.roi_supported && info->roi_info.roi_switch) {
		roi_info.op_action = TS_ACTION_WRITE;
		ret = brl_roi_switch(&roi_info);
		if (ret < 0)
			TS_LOG_ERR("%s, set_roi_switch failed\n", __func__);
	}

	if (glove_info.glove_supported && info->glove_info.glove_switch) {
		glove_info.op_action = TS_ACTION_WRITE;
		ret = brl_glove_switch(&glove_info);
		if (ret < 0)
			TS_LOG_ERR("Failed to set glove switch%u, err: %d\n",
				info->glove_info.glove_switch, ret);
	}

	if (charger_info.charger_supported) {
		charger_info.op_action = TS_ACTION_WRITE;
		ret = brl_charger_switch(&charger_info);
		if (ret < 0)
			TS_LOG_ERR("Failed to set charger switch%u, err: %d\n",
				info->charger_info.charger_switch, ret);
	}
	if (info->holster_info.holster_supported &&
		info->holster_info.holster_switch) {
		info->holster_info.op_action = TS_ACTION_WRITE;
		ret = brl_holster_switch(&(info->holster_info));
		if (ret < 0)
			TS_LOG_ERR("Failed to set holster switch:%u,err: %d\n",
				info->holster_info.holster_switch, ret);
	}

	brl_touch_switch();
	TS_LOG_INFO("reset touch switch\n");

	TS_LOG_INFO(" glove_switch %u, roi_switch %u, charger_switch %u, holster switch %u\n",
		info->glove_info.glove_switch, info->roi_info.roi_switch,
		info->charger_info.charger_switch, info->holster_info.holster_switch);
	return ret;
}

/**
 * brl_parse_dts - parse gtx8 private properties
 */
static int brl_parse_dts(void)
{
	struct brl_ts_data *ts = brl_ts;
	struct device_node *device = ts->pdev->dev.of_node;
	struct ts_kit_device_data *chip_data = ts->dev_data;
	int ret;

	ret = of_property_read_u32(device, GTP_X_MAX_MT, &ts->max_x);
	if (ret) {
		TS_LOG_ERR("%s: Get x_max_mt failed", __func__);
		ret = -EINVAL;
		goto err;
	}
	ret = of_property_read_u32(device, GTP_Y_MAX_MT, &ts->max_y);
	if (ret) {
		TS_LOG_ERR("%s: Get y_max_mt failed", __func__);
		ret = -EINVAL;
		goto err;
	}
	ret = of_property_read_u32(device, GTP_ALGO_ID,
		&chip_data->algo_id);
	if (ret) {
		TS_LOG_ERR("%s: Get algo id failed", __func__);
		ret = -EINVAL;
		goto err;
	}
	ret = of_property_read_u32(device, GTP_TOOL_SUPPORT,
		&ts->tools_support);
	if (ret) {
		ts->tools_support = 0;
		TS_LOG_INFO("%s: tools_support use default value", __func__);
	}

	ret = of_property_read_u32(device, "support_notice_aft_gesture_mode",
		&(chip_data->support_notice_aft_gesture_mode));
	if (ret)
		chip_data->support_notice_aft_gesture_mode = 0;
	TS_LOG_INFO("support_notice_aft_gesture_mode = %u\n",
		chip_data->support_notice_aft_gesture_mode);

	ret = of_property_read_u32(device, "tui_special_feature_support",
		&chip_data->tui_special_feature_support);
	if (ret)
		chip_data->tui_special_feature_support = 0;
	TS_LOG_INFO("get tui_special_feature_support = %u\n",
		chip_data->tui_special_feature_support);

	ret = of_property_read_string(device, "vendor_name",
		&chip_data->vendor_name);
	if (ret)
		chip_data->vendor_name = "null";

	strncpy(chip_data->module_name, GOODIX_VENDER_NAME, MAX_STR_LEN - 1);
	TS_LOG_INFO("%s: end", __func__);
	ret = NO_ERR;
err:
	return ret;
}

static int brl_parse_cfg_data(const struct firmware *cfg_bin,
	char *cfg_type, u8 *cfg, int *cfg_len, u8 sid)
{
	int i;
	int config_status = 0;
	int one_cfg_count;
	u8 bin_group_num;
	u8 bin_cfg_num;
	u16 cfg_checksum;
	u16 checksum = 0;
	u8 sid_is_exist;

	if (get_u32(cfg_bin->data) + BIN_CFG_START_LOCAL != cfg_bin->size) {
		TS_LOG_ERR("%s:Bad firmware!", __func__);
		goto exit;
	}

	/* check firmware's checksum */
	cfg_checksum = get_u16(&cfg_bin->data[4]);
	for (i = BIN_CFG_START_LOCAL; i < (int)(cfg_bin->size); i++)
		checksum += cfg_bin->data[i];
	if ((checksum) != cfg_checksum) {
		TS_LOG_ERR("%s:Bad firmware!", __func__);
		goto exit;
	}
	/* check head end  */

	bin_group_num = cfg_bin->data[MODULE_NUM];
	bin_cfg_num = cfg_bin->data[CFG_NUM];
	TS_LOG_INFO("%s:bin_group_num=%u, bin_cfg_num=%u\n",
		__func__, bin_group_num, bin_cfg_num);

	if (!strncmp(cfg_type, GTX8_TEST_CONFIG,
		strlen(GTX8_TEST_CONFIG))) {
		config_status = 0;
	} else if (!strncmp(cfg_type, GTX8_NORMAL_CONFIG,
		strlen(GTX8_NORMAL_CONFIG))) {
		config_status = 1;
	} else if (!strncmp(cfg_type, GTX8_NORMAL_NOISE_CONFIG,
		strlen(GTX8_NORMAL_NOISE_CONFIG))) {
		config_status = 2;
	} else if (!strncmp(cfg_type, GTX8_GLOVE_CONFIG,
		strlen(GTX8_GLOVE_CONFIG))) {
		config_status = 3;
	} else if (!strncmp(cfg_type, GTX8_GLOVE_NOISE_CONFIG,
		strlen(GTX8_GLOVE_NOISE_CONFIG))) {
		config_status = 4;
	} else if (!strncmp(cfg_type, GTX8_HOLSTER_CONFIG,
		strlen(GTX8_HOLSTER_CONFIG))) {
		config_status = 5;
	} else if (!strncmp(cfg_type, GTX8_HOLSTER_NOISE_CONFIG,
		strlen(GTX8_HOLSTER_NOISE_CONFIG))) {
		config_status = 6;
	} else if (!strncmp(cfg_type, GTX8_NOISE_TEST_CONFIG,
		strlen(GTX8_NOISE_TEST_CONFIG))) {
		config_status = 7;
	} else {
		TS_LOG_ERR("%s: invalid config text field", __func__);
		goto exit;
	}

	for (i = 0; i < bin_group_num * bin_cfg_num; i++) {
		/* find cfg's sid in cfg.bin */
		if (sid == (cfg_bin->data[CFG_HEAD_BYTES +
			i * CFG_INFO_BLOCK_BYTES])) {
			sid_is_exist = GTX8_EXIST;
			if (config_status == (cfg_bin->data[CFG_HEAD_BYTES +
				1 + i * CFG_INFO_BLOCK_BYTES])) {
				one_cfg_count =
				get_u16(&cfg_bin->data[CFG_HEAD_BYTES +
					2 + i * CFG_INFO_BLOCK_BYTES]);
				memcpy(cfg, &cfg_bin->data[CFG_HEAD_BYTES +
					bin_group_num * bin_cfg_num *
					CFG_INFO_BLOCK_BYTES +
					i * one_cfg_count],
					one_cfg_count);
				*cfg_len = one_cfg_count;
				TS_LOG_INFO("one_cfg_count=%d", one_cfg_count);

				break;
			}
		}
	}

	if (i >= bin_group_num * bin_cfg_num) {
		TS_LOG_ERR("%s:(not find config ,config_status: %d)",
			__func__, config_status);
		goto exit;
	}

	return NO_ERR;
exit:
	return RESULT_ERR;
}

static int brl_get_cfg_data(const struct firmware *cfg_bin,
	char *config_name, struct brl_ts_config *config)
{
	u8 *cfg_data = NULL;
	int cfg_len = 0;
	int ret = NO_ERR;

	cfg_data = kzalloc(GOODIX_CFG_MAX_SIZE, GFP_KERNEL);
	if (cfg_data == NULL) {
		TS_LOG_ERR("Memory allco err");
		goto exit;
	}

	config->initialized = false;
	mutex_init(&config->lock);
	config->reg_base = GTP_REG_CFG_ADDR;

	/* parse config data */
	ret = brl_parse_cfg_data(cfg_bin, config_name, cfg_data,
		&cfg_len, 0);
	if (ret < 0) {
		TS_LOG_ERR("%s: parse %s data failed", __func__, config_name);
		ret = -EINVAL;
		goto exit;
	}

	TS_LOG_INFO("%s: %s  version:%d , size:%d",
		__func__, config_name, cfg_data[34], cfg_len); /* 34:version offset */
	memcpy(config->data, cfg_data, cfg_len);
	config->length = cfg_len;

	strncpy(config->name, config_name, MAX_STR_LEN);
	config->initialized = true;

exit:
	kfree(cfg_data);
	cfg_data = NULL;
	return ret;
}

static int brl_get_cfg_parms(struct brl_ts_data *ts, const char *filename)
{
	int ret;
	const struct firmware *cfg_bin = NULL;

	ret = request_firmware(&cfg_bin, filename, &brl_ts->pdev->dev);
	if (ret < 0) {
		TS_LOG_ERR("%s:Request config firmware failed - %s-%d",
			__func__, filename, ret);
		goto exit;
	}

	if (cfg_bin->data == NULL) {
		TS_LOG_ERR("%s:Bad firmware!\n",
			__func__);
		goto exit;
	}

	TS_LOG_INFO("%s: cfg_bin_size", __func__);
	/* parse normal config data */
	ret = brl_get_cfg_data(cfg_bin, GTX8_NORMAL_CONFIG,
		&ts->normal_cfg);
	if (ret < 0)
		TS_LOG_ERR("%s: Failed to parse normal_config data:%d",
			__func__, ret);
	ret = brl_get_cfg_data(cfg_bin, GTX8_TEST_CONFIG, &ts->test_cfg);
	if (ret < 0)
		TS_LOG_ERR("%s: Failed to parse test_config data:%d",
			__func__, ret);
	/* parse normal noise config data */
	ret = brl_get_cfg_data(cfg_bin, GTX8_NORMAL_NOISE_CONFIG,
		&ts->normal_noise_cfg);
	if (ret < 0)
		TS_LOG_ERR("%s: Failed to parse normal_noise_config data",
			__func__);
	/* parse noise test config data */
	ret = brl_get_cfg_data(cfg_bin, GTX8_NOISE_TEST_CONFIG,
		&ts->noise_test_cfg);
	if (ret < 0) {
		memcpy(&ts->noise_test_cfg, &ts->normal_cfg,
			sizeof(ts->noise_test_cfg));
		TS_LOG_ERR("%s: Failed to parse noise_test_config data",
			__func__);
	}
exit:
	if (cfg_bin != NULL) {
		release_firmware(cfg_bin);
		cfg_bin = NULL;
	}
	return NO_ERR;
}

static int brl_init_configs(struct brl_ts_data *ts)
{
	char filename[GTX8_FW_NAME_LEN + 1] = {0};

	/* get cfg_bin_name */

	snprintf(filename, GTX8_FW_NAME_LEN, "ts/%s_cfg.bin",
		brl_ts->fw_name_prefix);
	TS_LOG_INFO("%s: filename = %s", __func__, filename);
	return brl_get_cfg_parms(ts, filename);
}

static int brl_i2c_communicate_check(void)
{
	u32 reg = GOODIX_REG_ESD;
	u8 val;
	int ret;

	ret = brl_i2c_read(reg, &val, 1); /* esd addr */
	return ret;
}

static int brl_sub_board_connect_check(
	struct ts_kit_platform_data *data)
{
	int gpio_value;
	int retval;
	int i;

	if (data->i2c_driver_type != I3C_MASTER_I2C_DRIVER)
		return 0;

	TS_LOG_INFO("%s connect_check_gpio = %d\n", __func__,
		data->connect_check_gpio);
	for (i = 0; i < 3; i++) { /* 3: retry 3 times*/
		gpio_value = gpio_get_value(data->connect_check_gpio);
		TS_LOG_ERR("%s connect_check_gpio gpio_value:%d\n",
			__func__, gpio_value);
		if (gpio_value == 0)
			break;
		mdelay(3); /* 3: 3ms */
	}
	/* when gpio_value :1 disconnect sub board return -ENODEV */
	if (gpio_value != 0) {
		TS_LOG_ERR("%s sub board not connect gpio:%d gpio_value:%d\n",
			__func__, data->connect_check_gpio, gpio_value);
		retval = -ENODEV;
	} else {
	/* when gpio_value :0 connect sub board return 0 */
		retval = 0;
	}
	return retval;
}

static int brl_chip_detect(struct ts_kit_platform_data *pdata)
{
	int ret;
	int retry = GTX8_RETRY_NUM_3;
	TS_LOG_INFO("Chip detect\n");

	brl_ts->pdev = pdata->ts_dev;
	brl_ts->dev_data->ts_platform_data = pdata;
	brl_ts->pdev->dev.of_node = pdata->chip_data->cnode;
	brl_ts->ops.i2c_read = brl_i2c_read;
	brl_ts->ops.i2c_write = brl_i2c_write;
	brl_ts->ops.chip_reset = brl_chip_reset;
	brl_ts->ops.send_cmd = brl_send_cmd;
	brl_ts->ops.send_cfg = brl_send_config;
	brl_ts->ops.read_cfg = brl_read_config,
	brl_ts->ops.read_version = brl_read_version;
	brl_ts->ops.feature_resume = brl_feature_resume;
	brl_ts->ops.get_file_config_id = brl_get_file_config_id;

	/* Do *NOT* remove these logs */
	TS_LOG_INFO("Driver Version: %s\n", GTP_DRIVER_VERSION);

	ret = brl_prepare_parse_dts(pdata);
	if (ret < 0) {
		TS_LOG_ERR("%s: prepare_parse_dts failed\n", __func__);
		goto out;
	}
	ret = brl_power_init();
	if (ret < 0) {
		TS_LOG_ERR("brl_power_init error %d\n", ret);
		goto out;
	}
	if (ts_is_factory() && pdata->support_connect_check_gpio == 1) {
		if (brl_sub_board_connect_check(pdata) < 0) {
			TS_LOG_ERR("%s: sub_board not connect\n", __func__);
			ret = -ENODEV;
			goto err_regulator;
		}
	}
	ret = brl_pinctrl_init();
	if (ret < 0) {
		TS_LOG_ERR("pinctrl_init fail");
		goto err_regulator;
	}
	/* power on */
	ret = brl_power_switch(SWITCH_ON);
	if (ret < 0) {
		TS_LOG_ERR("failed to enable power, ret = %d\n", ret);
		goto err_pinctrl_init;
	}
	msleep(100); /* delay 100ms */
	/* reset chip */
	while (retry) {
		ret = brl_i2c_communicate_check();
		if (ret == 0)
			break;
		retry--;
		brl_chip_reset();
		if (ret < 0)
			TS_LOG_ERR("failed to chip_reset, ret = %d\n", ret);
	}
	if (ret != 0)
		goto err_power_on;
	strncpy(brl_ts->dev_data->chip_name, GOODIX_VENDER_NAME,
		MAX_STR_LEN - 1);
	/* set feature switch status after chip reset */
	brl_ts->after_resume_done = true;

	return ret;
err_power_on:
	brl_power_switch(SWITCH_OFF);
err_pinctrl_init:
	brl_pinctrl_release();
err_regulator:
	brl_power_release();
out:
	return ret;
}

static int brl_ts_roi_init(struct brl_ts_roi *roi)
{
	u32 roi_bytes;
	struct brl_ts_data *ts = brl_ts;

	/* the following param must be modified */
	roi->roi_cols = ROI_COLS;
	roi->roi_rows = ROI_ROWS;
	ts->brl_roi_data_add = ROI_STA_REG;

	roi_bytes = (roi->roi_rows * roi->roi_cols + 1) * sizeof(*roi->rawdata);
	roi->rawdata = kzalloc(roi_bytes + ROI_HEAD_LEN, GFP_KERNEL);
	if (!roi->rawdata) {
		TS_LOG_ERR("%s: Failed to alloc memory for roi\n", __func__);
		return -ENOMEM;
	}

	roi->roi_mat_src.data = kzalloc((ROI_COLS * ROI_COLS + 1) *
		sizeof(short), GFP_KERNEL);
	if (!roi->roi_mat_src.data) {
		TS_LOG_ERR("failed alloc src_roi memory\n");
		kfree(roi->rawdata);
		roi->rawdata = NULL;
		return -ENOMEM;
	}
	roi->roi_mat_dst.data = kzalloc((ROI_COLS * ROI_COLS + 1) *
		sizeof(short), GFP_KERNEL);
	if (!roi->roi_mat_dst.data) {
		TS_LOG_ERR("failed alloc dst_roi memory\n");
		kfree(roi->roi_mat_src.data);
		roi->roi_mat_src.data = NULL;
		kfree(roi->rawdata);
		roi->rawdata = NULL;
		return -ENOMEM;
	}

	mutex_init(&roi->mutex);

	TS_LOG_INFO("%s: ROI init,rows:%d,cols:%d\n",
		__func__, roi->roi_rows, roi->roi_cols);

	return NO_ERR;
}

static int brl_get_fw_prefix(void)
{
	char joint_chr = '_';
	char *fwname = brl_ts->fw_name_prefix;
	struct ts_kit_platform_data *ts_platform_data = NULL;

	ts_platform_data = brl_ts->dev_data->ts_platform_data;
	brl_get_project_id(brl_ts);
	if ((strlen(ts_platform_data->product_name) +
		strlen(brl_ts->project_id) +
		2 * sizeof(char)) > MAX_STR_LEN) {
		TS_LOG_ERR("%s invalid fwname\n", __func__);
		return -EINVAL;
	}

	strncat(fwname, ts_platform_data->product_name, MAX_STR_LEN);
	strncat(fwname, &joint_chr, sizeof(char));
	strncat(fwname, brl_ts->project_id, MAX_STR_LEN);
	TS_LOG_INFO("%s fw name prefix:%s", __func__, fwname);

	return 0;
}

static int brl_glove_switch(struct ts_glove_info *info)
{
	int ret = NO_ERR;
	struct brl_ts_cmd cfg_cmd;

	if (!info || !info->glove_supported) {
		TS_LOG_ERR("%s: info is Null or no support glove mode\n",
			__func__);
		return -EINVAL;
	}

	if (brl_ts->after_resume_done == false) {
		TS_LOG_ERR("%s: ic in suspend,return directly\n", __func__);
		return 0;
	}

	switch (info->op_action) {
	case TS_ACTION_READ:
		TS_LOG_INFO("%s: invalid glove switch%u action: TS_ACTION_READ\n",
			__func__, info->glove_switch);
		break;
	case TS_ACTION_WRITE:
		TS_LOG_INFO("glove enable:%d\n", info->glove_switch);
		if (info->glove_switch) {
			cfg_cmd.len = CONFIG_CMD_LEN;
			cfg_cmd.cmd = GTX8_CMD_GLOVE_ENBALE;
			ret = brl_send_cmd(&cfg_cmd);
			if (ret)
				TS_LOG_ERR("failed enable glove cmd %d\n", ret);
		} else {
			cfg_cmd.len = CONFIG_CMD_LEN;
			cfg_cmd.cmd = GTX8_CMD_GLOVE_DISBALE;
			ret = brl_send_cmd(&cfg_cmd);
			if (ret)
				TS_LOG_ERR("failed disable glove cmd %d\n", ret);
		}

		if (ret < 0)
			TS_LOG_ERR("%s:set glove switch %u, failed : %d\n",
				__func__, info->glove_switch, ret);
		break;
	default:
		TS_LOG_ERR("%s: invalid switch status: %d\n",
			__func__, info->glove_switch);
		ret = -EINVAL;
		break;
	}

	return ret;
}

#define CHARGER_MODE_ON 0x10
#define CHARGER_MODE_OFF 0x11
static int brl_charger_switch(struct ts_charger_info *info)
{
	int ret = NO_ERR;
	struct brl_ts_cmd  temp_cmd;
	struct ts_feature_info *feature_info = NULL;

	if (!info || !info->charger_supported || !brl_ts ||
		!brl_ts->dev_data || !brl_ts->dev_data->ts_platform_data) {
		TS_LOG_ERR("%s: info is Null or no support charger mode\n",
			__func__);
		ret = -EINVAL;
		return ret;
	}
	feature_info = &brl_ts->dev_data->ts_platform_data->feature_info;

	switch (info->op_action) {
	case TS_ACTION_WRITE:
		TS_LOG_INFO("charger enable:%d\n", info->charger_switch);
		if (info->charger_switch)
			temp_cmd.cmd = CHARGER_MODE_ON;
		else
			temp_cmd.cmd = CHARGER_MODE_OFF;
		TS_LOG_INFO("send charger cmd:0x%02x\n", temp_cmd.cmd);
		temp_cmd.len = CONFIG_CMD_LEN;
		ret = brl_send_cmd(&temp_cmd);
		if (ret)
			TS_LOG_ERR("send charger cmd failed, cmd:0x%02x\n",
				temp_cmd.cmd);
		break;
	case TS_ACTION_READ:
		TS_LOG_INFO("%s:charger_switch read :%d\n",
			__func__, info->charger_switch);
		break;
	default:
		TS_LOG_INFO("%s: invalid charger_switch action: %d\n",
			__func__, info->op_action);
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int brl_holster_switch(struct ts_holster_info *info)
{
	int ret = NO_ERR;
	struct brl_ts_cmd cfg_cmd;

	if (!info || !info->holster_supported) {
		TS_LOG_ERR("%s: info is Null or no support holster mode\n",
			__func__);
		ret = -EINVAL;
		return ret;
	}

	if (brl_ts->after_resume_done == false) {
		TS_LOG_ERR("%s: ic in suspend,return directly\n", __func__);
		return 0;
	}

	switch (info->op_action) {
	case TS_ACTION_READ:
		TS_LOG_INFO("%s: invalid holster switch%u action: TS_ACTION_READ\n",
			__func__, info->holster_switch);
		break;
	case TS_ACTION_WRITE:
		TS_LOG_INFO("holster enable:%d\n", info->holster_switch);
		if (info->holster_switch) {
			cfg_cmd.len = CONFIG_CMD_LEN;
			cfg_cmd.cmd = GTX8_CMD_HOLSTER_ENBALE;
			ret = brl_send_cmd(&cfg_cmd);
			if (ret)
				TS_LOG_ERR("failed enbale holster  cmd %d\n", ret);
		} else {
			cfg_cmd.len = CONFIG_CMD_LEN;
			cfg_cmd.cmd = GTX8_CMD_HOLSTER_DISBALE;
			ret = brl_send_cmd(&cfg_cmd);
			if (ret)
				TS_LOG_ERR("failed disable holster cmd %d\n", ret);
		}

		if (ret < 0)
			TS_LOG_ERR("%s:set holster switch%u, failed : %d\n",
				__func__, info->holster_switch, ret);
		break;
	default:
		TS_LOG_ERR("%s: invalid switch status: %d\n",
			__func__, info->holster_switch);
		ret = -EINVAL;
		break;
	}

	return ret;
}

#define GAME_MODE_ON 0x1A
#define GAME_MODE_OFF 0x1B
#define GAME_CMD_LEN 4
static void brl_game_switch(int soper)
{
	int ret = -EINVAL;
	struct brl_ts_cmd temp_cmd;

	temp_cmd.len = GAME_CMD_LEN;

	switch (soper) {
	case TS_SWITCH_GAME_ENABLE:
		temp_cmd.cmd = GAME_MODE_ON;
		ret = brl_send_cmd(&temp_cmd);
		if (ret) {
			TS_LOG_ERR("game switch on error \n");
		} else {
			brl_ts->game_switch = true;
			TS_LOG_INFO("[%s] game switch on:%d \n",
				__func__, brl_ts->game_switch);
		}
		break;
	case TS_SWITCH_GAME_DISABLE:
		temp_cmd.cmd = GAME_MODE_OFF;
		ret = brl_send_cmd(&temp_cmd);
		if (ret) {
			TS_LOG_ERR("game switch off error \n");
		} else {
			brl_ts->game_switch = false;
			TS_LOG_INFO("[%s] game switch off:%d\n",
				__func__, brl_ts->game_switch);
		}
		break;
	default:
		TS_LOG_ERR("soper unknown:%d, invalid\n", soper);
		break;
	}
}

#define GOODIX_SWITCH_TYPE_SCENE TS_SWITCH_TYPE_SCENE
#define GOODIX_SWITCH_TYPE_GAME 5
#define MAX_TYPE_LEN 3
static void brl_touch_switch(void)
{
	unsigned long get_value = 0;
	char *ptr_begin = NULL;
	char *ptr_end = NULL;
	char in_data[MAX_STR_LEN] = {0};
	int len = 0;
	int error = 0;
	u8 stype = 0;
	u8 soper = 0;

	TS_LOG_INFO("%s +\n", __func__);
	if (brl_ts->after_resume_done == false) {
		TS_LOG_ERR("%s: ic in suspend,return directly\n", __func__);
		return;
	}
	if (!(brl_ts->dev_data->touch_switch_flag & TS_SWITCH_TYPE_DOZE) &&
		!(brl_ts->dev_data->touch_switch_flag & TS_SWITCH_TYPE_GAME) &&
		!(brl_ts->dev_data->touch_switch_flag & TS_SWITCH_TYPE_SCENE)) {
		TS_LOG_ERR("touch switch not supprot\n");
		goto out;
	}

	/* SWITCH_OPER,ENABLE_DISABLE,PARAM */
	memcpy(in_data, brl_ts->dev_data->touch_switch_info, MAX_STR_LEN - 1);
	TS_LOG_INFO("in_data:%s\n", in_data);

	/* get switch type */
	ptr_begin = in_data;
	ptr_end = strstr(ptr_begin, ",");
	if (!ptr_end) {
		TS_LOG_ERR("%s get stype fail\n", __func__);
		goto out;
	}
	len = ptr_end - ptr_begin;
	if ((len <= 0) || (len > MAX_TYPE_LEN)) {
		TS_LOG_ERR("%s stype len error\n", __func__);
		goto out;
	}
	*ptr_end = 0;
	error = strict_strtoul(ptr_begin, 0, &get_value);
	if (error) {
		TS_LOG_ERR("strict_strtoul return invaild :%d\n", error);
		goto out;
	} else {
		stype = (unsigned char)get_value;
		TS_LOG_INFO("%s get stype:%u\n", __func__, stype);
	}

	/* get switch operate */
	ptr_begin = ptr_end + 1;
	if (!ptr_begin) {
		TS_LOG_ERR("%s get soper fail\n", __func__);
		goto out;
	}
	ptr_end = strstr(ptr_begin, ",");
	if (!ptr_end) {
		TS_LOG_ERR("%s get soper fail\n", __func__);
		goto out;
	}
	len = ptr_end - ptr_begin;
	if (len <= 0 || len > MAX_TYPE_LEN) {
		TS_LOG_ERR("%s soper len error\n", __func__);
		goto out;
	}
	*ptr_end = 0;
	error = strict_strtoul(ptr_begin, 0, &get_value);
	if (error) {
		TS_LOG_ERR("strict_strtoul return invaild :%d\n", error);
		goto out;
	} else {
		soper = (unsigned char)get_value;
		TS_LOG_INFO("%s get soper:%u\n", __func__, soper);
	}

	if (stype == GOODIX_SWITCH_TYPE_SCENE)
		TS_LOG_INFO("WARNING: not support scene mode");
	else if (stype == GOODIX_SWITCH_TYPE_GAME)
		brl_game_switch(soper);
	else
		TS_LOG_ERR("stype not GAME or pen respons :%d, invalid\n", stype);
out:
	return;
}

#if defined(CONFIG_TEE_TUI)
extern struct ts_tui_data tee_tui_data;
#endif
static int brl_chip_init(void)
{
	struct brl_ts_data *ts = brl_ts;
	int ret;
	struct ts_roi_info *roi_info = NULL;

	roi_info = &ts->dev_data->ts_platform_data->feature_info.roi_info;

	brl_ts->rawdiff_mode = false;
	brl_ts->suspend_status = false;
	init_completion(&roi_data_done);
	/* obtain brl dt properties */
	ret = brl_parse_dts();
	if (ret < 0) {
		TS_LOG_ERR("%s: parse_dts failed\n", __func__);
		return ret;
	}

	if (roi_info->roi_supported) {
		ret = brl_ts_roi_init(&ts->roi);
		if (ret < 0) {
			TS_LOG_ERR("%s: brl_ts_roi_init fail!\n", __func__);
			return ret;
		}
	}
	ret = brl_get_fw_prefix();
	if (ret < 0)
		TS_LOG_INFO("%s: get_fw_prefix failed\n", __func__);

	/* init config data, normal/glove/hoslter config data */
	ret = brl_init_configs(ts);
	if (ret < 0)
		TS_LOG_INFO("%s: init config failed\n", __func__);
	/* GTP debug node */
	if (ts->tools_support) {
		ret = brl_init_tool_node();
		if (ret < 0)
			TS_LOG_ERR("%s : init_tool_node failed\n", __func__);
	}

#if defined CONFIG_TEE_TUI
	strncpy(tee_tui_data.device_name, "brl_gtx8",
		strlen("brl_gtx8")); /* strlen device_name is 11 more then 8 */
	tee_tui_data.device_name[strlen("brl_gtx8")] = '\0';
	tee_tui_data.tui_special_feature_support =
		ts->dev_data->ts_platform_data->chip_data->tui_special_feature_support;
	if ((tee_tui_data.tui_special_feature_support & TP_TUI_NEW_IRQ_MASK) ==
		TP_TUI_NEW_IRQ_SUPPORT) {
		tee_tui_data.tui_irq_num =
			ts->dev_data->ts_platform_data->irq_gpio;
		tee_tui_data.tui_irq_gpio =
			ts->dev_data->ts_platform_data->tui_another_irq_gpio_num;
	}
	TS_LOG_INFO("%s tui_irq_num:%u tui_irq_gpio:%u\n", __func__,
		tee_tui_data.tui_irq_num, tee_tui_data.tui_irq_gpio);
#endif
	ret = brl_get_ic_info(&brl_ts->ic_info);
	if (ret < 0)
		TS_LOG_ERR("%s: get ic info failed", __func__);
	TS_LOG_INFO("%s finish\n", __func__);
	return ret;
}

static int brl_input_config(struct input_dev *input_dev)
{
	struct brl_ts_data *ts = brl_ts;

	if (ts == NULL)
		return -ENODEV;

	__set_bit(EV_SYN, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);
	__set_bit(BTN_TOOL_FINGER, input_dev->keybit);
	__set_bit(TS_DOUBLE_CLICK, input_dev->keybit);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);

#ifdef INPUT_TYPE_B_PROTOCOL
	input_mt_init_slots(input_dev, GTP_MAX_TOUCH * 2 + 1, INPUT_MT_DIRECT);
#endif

	/* set ABS_MT_TOOL_TYPE */
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, ts->max_x, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, ts->max_y, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0,
		GTX8_ABS_MAX_VALUE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0,
		GTX8_ABS_MAX_VALUE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR, 0,
		GTX8_ABS_MAX_VALUE, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0,
		GTP_MAX_TOUCH, 0, 0);
	return 0;
}

/*
 * brl_fw_update_sd - update firmware from sdcard
 *  firmware path should be '/sdcard/gtx8_fw.bin'
 */
static int brl_fw_update_sd(void)
{
	int ret;

	TS_LOG_INFO("%s: start\n", __func__);
	TS_LOG_INFO("before the update. normal_cfg_version:0x%02x\n",
		brl_ts->normal_cfg.data[34]);
	if (brl_ts->after_resume_done == false) {
		TS_LOG_ERR("%s: ic in suspend,return directly\n", __func__);
		return 0;
	}

	brl_read_version(&brl_ts->hw_info);
	brl_get_cfg_parms(brl_ts, GTX8_CONFIG_FW_SD_NAME);
	memset(brl_update_ctrl.fw_name, 0, sizeof(brl_update_ctrl.fw_name));
	strncpy(brl_update_ctrl.fw_name, GTX8_FW_SD_NAME, GTX8_FW_NAME_LEN);
	brl_update_ctrl.force_update = true;
	TS_LOG_INFO("%s: file_name = %s\n", __func__, brl_update_ctrl.fw_name);
	ret = brl_update_firmware();
	brl_update_ctrl.force_update = false;
	brl_read_version(&brl_ts->hw_info);
	brl_get_ic_info(&brl_ts->ic_info);
	brl_feature_resume(brl_ts);
	brl_status_resume();
	TS_LOG_INFO("after the update. normal_cfg_version:0x%02x\n",
		brl_ts->normal_cfg.data[34]);

	return ret;
}

static int brl_request_handler(struct brl_ts_data *ts, u8 *pre_buf)
{
	int ret = -EINVAL;

	/* 2 : request event flag */
	switch (pre_buf[2]) {
	case REQUEST_CONFIG:
		ret = brl_feature_resume(ts);
		break;
	case REQUEST_BAKREF:
		TS_LOG_ERR("can't support request type REQUEST_BAKREF\n");
		break;
	case REQUEST_RESET:
		ret = brl_chip_reset();
		break;
	case REQUEST_MAINCLK:
		TS_LOG_ERR("can't support request type REQUEST_MAINCLK\n");
		break;
	case REQUEST_RELOADFW:
		brl_update_ctrl.force_update = true;
		ret = brl_fw_update_boot(brl_update_ctrl.fw_name);
		brl_update_ctrl.force_update = false;
		break;
	default:
		TS_LOG_ERR("Unknown hw request:0x%02x\n", pre_buf[2]);
		break;
	}

	if (ret)
		TS_LOG_ERR("failed to handle request\n");
	else
		TS_LOG_INFO("success to handle request\n");
	return ret;
}

#define GOODIX_DEBUG_INFO_ADDR 0x103CE
#define GOODIX_DEBUG_INFO_LENGTH 6
#define GOODIX_CHECKSUM_LENGTH 2
static int brl_touch_handler(struct brl_ts_data  *ts,
	struct ts_fingers *info, u8 *pre_buf, u32 pre_buf_len)
{
	int ret = 0;
	u32 touch_num = pre_buf[2] & 0x0f;
	u8 buffer[BYTES_PER_COORD * GTP_MAX_TOUCH + GOODIX_CHECKSUM_LENGTH] = {0};
	u8 edge_buf[BYTES_PER_EDGE * GTP_MAX_TOUCH] = {0};
	u8 debug_info_buf[GOODIX_DEBUG_INFO_LENGTH] = {0};
	static u8 pre_debug_info_buf[GOODIX_DEBUG_INFO_LENGTH];

	u32 reg = ts->ic_info.misc.touch_data_addr;
	unsigned int id;
	int x;
	int y;
	int w;
	u32 i;
	u8 finger_status;

	if (touch_num > GTP_MAX_TOUCH)
		return -EINVAL;
	cur_index = 0;

	memset(info, 0x00, sizeof(struct ts_fingers));

	reg += BYTES_PER_COORD;
	ret = brl_i2c_read(reg, buffer, touch_num *
		BYTES_PER_COORD + GOODIX_CHECKSUM_LENGTH);
	if (ret) {
		TS_LOG_ERR("get touch data failed\n");
		return ret;
	}

	if (touch_num && checksum_cmp(buffer, touch_num *
		BYTES_PER_COORD + GOODIX_CHECKSUM_LENGTH, CHECKSUM_MODE_U8_LE)) {
		TS_LOG_ERR("touch data checksum err");
		TS_LOG_ERR("touch data %*ph",
			touch_num * BYTES_PER_COORD +
			GOODIX_CHECKSUM_LENGTH, buffer);

		ret = -EINVAL;
		return ret;
	}

	reg = GOODIX_EDGE_ADDR;
	ret = brl_i2c_read(reg, edge_buf, touch_num * BYTES_PER_EDGE);
	if (ret) {
		TS_LOG_ERR("get edge data failed\n");
		return ret;
	}

	reg = GOODIX_DEBUG_INFO_ADDR;
	ret = brl_i2c_read(reg, debug_info_buf, GOODIX_DEBUG_INFO_LENGTH);
	if (ret) {
		TS_LOG_ERR("get edge data failed\n");
		return ret;
	}

	if ((debug_info_buf[0] != pre_debug_info_buf[0]) ||
		(debug_info_buf[1] != pre_debug_info_buf[1]))
		TS_LOG_INFO("debug info[0~5] : %x %x %x %x %x %x",
			debug_info_buf[0], debug_info_buf[1], debug_info_buf[2],
			debug_info_buf[3], debug_info_buf[4], debug_info_buf[5]);

	memcpy(pre_debug_info_buf, debug_info_buf, sizeof(debug_info_buf));
	for (i = 0; i < touch_num; i++) {
		id = (buffer[i * BYTES_PER_COORD] >> 4) & 0x0f;
		finger_status = buffer[i * BYTES_PER_COORD] & 0x0f;
		x = buffer[i * BYTES_PER_COORD + 2] |
			(buffer[i * BYTES_PER_COORD + 3] << 8);
		y = buffer[i * BYTES_PER_COORD + 4] |
			(buffer[i * BYTES_PER_COORD + 5] << 8);
		w = buffer[i * BYTES_PER_COORD + 6] |
			(buffer[i * BYTES_PER_COORD + 7] << 8);

		info->fingers[id].wx = edge_buf[i * BYTES_PER_EDGE + 0];
		info->fingers[id].wy = edge_buf[i * BYTES_PER_EDGE + 1];
		info->fingers[id].ewx = edge_buf[i * BYTES_PER_EDGE + 2];
		info->fingers[id].ewy = edge_buf[i * BYTES_PER_EDGE + 3];
		info->fingers[id].xer = edge_buf[i * BYTES_PER_EDGE + 4];
		info->fingers[id].yer = edge_buf[i * BYTES_PER_EDGE + 5];

#if GOODIX_EXCHANGE_X
		info->fingers[id].x = ts->max_x - x;
#else
		info->fingers[id].x = x;
#endif

#if GOODIX_EXCHANGE_Y
		info->fingers[id].y = ts->max_y - y;
#else
		info->fingers[id].y = y;
#endif
		info->fingers[id].major = w;
		info->fingers[id].minor = w;
		info->fingers[id].pressure = w;
		info->fingers[id].status = finger_status;
		cur_index |= 1 << id;
	}
	cur_finger_number = touch_num;
	info->cur_finger_number = touch_num;
	return ret;
}

static int brl_gesture_handler(struct brl_ts_data *ts,
	struct ts_fingers *info, u8 *pre_buf, int buf_len)
{
	int ret = NO_ERR;
	unsigned char ts_state;

	ts_state = atomic_read(&ts->dev_data->ts_platform_data->state);
	TS_LOG_DEBUG("%s: TS_WORK_STATUS :%u\n", __func__, ts_state);
	if (ts_state != TS_WORK_IN_SLEEP) {
		TS_LOG_DEBUG("%s: TS_WORK_STATUS %d is mismatch\n",
			__func__, ts_state);
		return -EINVAL;
	}

	/* 4:gesture flag */
	switch (pre_buf[4]) {
	case GESTURE_DOUBLE_CLICK:
		info->gesture_wakeup_value = TS_DOUBLE_CLICK;
		break;
	default:
		TS_LOG_ERR("unsupport gesture type\n");
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int brl_irq_bottom_half(struct ts_cmd_node *in_cmd,
				struct ts_cmd_node *out_cmd)
{
	struct brl_ts_data *ts = brl_ts;
	struct ts_fingers *ts_fingers = NULL;
	u8 sync_val[2] = {0};
	int ret;
	u32 reg = ts->ic_info.misc.touch_data_addr;
	u8 pre_buf[BYTES_PER_COORD] = {0};
	struct ts_easy_wakeup_info *easy_wakeup_info = NULL;

	ts_fingers = &out_cmd->cmd_param.pub_params.algo_param.info;
	out_cmd->command = TS_INVAILD_CMD;
	out_cmd->cmd_param.pub_params.algo_param.algo_order =
			ts->dev_data->algo_id;
	easy_wakeup_info = &ts->dev_data->easy_wakeup_info;
	if (ts->test_mode)
		return NO_ERR;

	ret = brl_i2c_read(reg, pre_buf, sizeof(pre_buf));
	if (ret < 0) {
		TS_LOG_ERR("%s: read event head failed\n", __func__);
		goto clear_flag;
	}

	if (checksum_cmp(pre_buf, BYTES_PER_COORD, CHECKSUM_MODE_U8_LE)) {
		TS_LOG_ERR("touch head checksum err");
		TS_LOG_ERR("touch head %*ph", BYTES_PER_COORD, pre_buf);
		goto clear_flag;
	}

	if (pre_buf[0] & GTX8_REQUEST_EVENT) {
		/* request event */
		brl_request_handler(ts, pre_buf);
		goto clear_flag;
	} else if (ts_kit_gesture_func && (pre_buf[0] & GTX8_GESTURE_EVENT) &&
			(easy_wakeup_info->sleep_mode == TS_GESTURE_MODE) &&
			(easy_wakeup_info->easy_wakeup_gesture != 0)) {
		/* gesture event */
		ret = brl_gesture_handler(ts, ts_fingers, pre_buf,
			sizeof(pre_buf));
		if (ret == 0)
			out_cmd->command = TS_INPUT_ALGO;
		else
			TS_LOG_ERR("%s: gesture_evt_handler failed\n",
				__func__);
		goto clear_flag;
	} else if (pre_buf[0] & GTX8_TOUCH_EVENT) {
		/* touch event */
		ret = brl_touch_handler(ts, ts_fingers, pre_buf,
			sizeof(pre_buf));
		if (ret == 0)
			out_cmd->command = TS_INPUT_ALGO;
		else
			TS_LOG_ERR("%s: touch_evt_handler failed\n", __func__);
	}

clear_flag:
	if (!ts->rawdiff_mode)
		brl_i2c_write(reg, sync_val, 2);
	return ret;
}

/**
 * brl_resume_chip_reset - LCD resume reset chip
 */
static void brl_resume_chip_reset(void)
{
	int reset_gpio = 0;

	reset_gpio = brl_ts->dev_data->ts_platform_data->reset_gpio;
	TS_LOG_INFO("%s: Chip reset", __func__);
	gpio_direction_output(reset_gpio, 0);
	udelay(2000); /* 2000: 2ms */
	gpio_direction_output(reset_gpio, 1);
}

/**
 * brl_fw_update_boot - update firmware while booting
 */
static int brl_fw_update_boot(char *file_name)
{
	int ret;

	if (!file_name) {
		TS_LOG_ERR("%s: Invalid file name", __func__);
		return -ENODEV;
	}

	memset(brl_update_ctrl.fw_name, 0, sizeof(brl_update_ctrl.fw_name));
	snprintf(brl_update_ctrl.fw_name, GTX8_FW_NAME_LEN, "ts/%s.img",
		brl_ts->fw_name_prefix);

	TS_LOG_INFO("%s: file_name = %s", __func__, brl_update_ctrl.fw_name);
	brl_update_firmware();
	brl_read_version(&brl_ts->hw_info);
	ret = brl_get_ic_info(&brl_ts->ic_info);
	if (ret < 0) {
		TS_LOG_ERR("%s: get ic info failed", __func__);
		return ret;
	}
	brl_feature_resume(brl_ts);
	brl_status_resume();
	ret = brl_get_project_id(brl_ts);
	if (ret < 0)
		TS_LOG_ERR("get project id failed");
	return ret;
}

static int brl_chip_get_info(struct ts_chip_info_param *info)
{
	struct brl_ts_data *ts = brl_ts;
	int len;

	if (!info) {
		TS_LOG_ERR("%s: info is null\n", __func__);
		return -EINVAL;
	}

	TS_LOG_INFO("%s: enter\n", __func__);

	memset(info->ic_vendor, 0, sizeof(info->ic_vendor));
	memset(info->mod_vendor, 0, sizeof(info->mod_vendor));
	memset(info->fw_vendor, 0, sizeof(info->fw_vendor));

	if ((!ts->dev_data->ts_platform_data->hide_plain_id) ||
		ts_is_factory()) {
		len = (sizeof(info->ic_vendor) - 1) > sizeof(GTX8_OF_NAME) ?
			sizeof(GTX8_OF_NAME) : (sizeof(info->ic_vendor) - 1);
		strncpy(info->ic_vendor, GTX8_OF_NAME, len);
	} else {
		len = (sizeof(info->ic_vendor) - 1) > sizeof(ts->project_id) ?
			sizeof(ts->project_id) : (sizeof(info->ic_vendor) - 1);
		strncpy(info->ic_vendor, ts->project_id, len);
	}
	len = (sizeof(info->mod_vendor) - 1) > strlen(ts->project_id) ?
		strlen(ts->project_id) : (sizeof(info->mod_vendor) - 1);
	strncpy(info->mod_vendor, ts->project_id, len);
	snprintf(info->fw_vendor, sizeof(info->fw_vendor),
		"%02X_%02X_%X", ts->hw_info.patch_vid[PATCH_VID_MINOR],
		ts->hw_info.patch_vid[PATCH_VID_MAJOR],
		ts->normal_cfg.data[NORMAL_CFG_DATA]);

	if (ts->dev_data->ts_platform_data->hide_plain_id &&
		(!ts_is_factory())) {
		/*
		 * The length of ic_vendor is CHIP_INFO_LENGTH * 2
		 * no overwriting occurs
		 */
		snprintf(info->ic_vendor, CHIP_INFO_LENGTH * 2,
			"TP-%s-%s", info->mod_vendor, info->fw_vendor);
		TS_LOG_INFO("info->ic_vendor :%s\n", info->ic_vendor);
	}
	TS_LOG_INFO("ic_vendor:%s\n", info->ic_vendor);
	TS_LOG_INFO("mod_vendor:%s\n", info->mod_vendor);
	TS_LOG_INFO("fw_vendor:%s\n", info->fw_vendor);

	return NO_ERR;
}

static int brl_chip_get_capacitance_test_type(
	struct ts_test_type_info *info)
{
	int retval = NO_ERR;

	if (!info) {
		TS_LOG_ERR("%s: info is Null\n", __func__);
		return -ENOMEM;
	}
	switch (info->op_action) {
	case TS_ACTION_READ:
		strncpy(info->tp_test_type,
			brl_ts->dev_data->tp_test_type,
			TS_CAP_TEST_TYPE_LEN - 1);
		TS_LOG_INFO("read_chip_get_test_type=%s\n",
			info->tp_test_type);
		break;
	case TS_ACTION_WRITE:
		break;
	default:
		TS_LOG_ERR("invalid status: %s", info->tp_test_type);
		retval = -EINVAL;
		break;
	}
	return retval;
}

#define ROI_SWITCH_ENABLE 1
#define ROI_SWITCH_DISABLE 0
static int brl_roi_switch(struct ts_roi_info *info)
{
	int ret = 0;

	if (!info || !info->roi_supported || !brl_ts) {
		TS_LOG_ERR("%s: info is Null or not supported roi mode\n",
			__func__);
		ret = -EINVAL;
		return ret;
	}

	if (brl_ts->after_resume_done == false) {
		TS_LOG_ERR("%s: ic in suspend,return directly\n", __func__);
		return 0;
	}

	switch (info->op_action) {
	case TS_ACTION_WRITE:
		if (info->roi_switch == ROI_SWITCH_ENABLE) {
			brl_ts->roi.enabled = true;
		} else if (info->roi_switch == ROI_SWITCH_DISABLE) {
			brl_ts->roi.enabled = false;
		} else {
			TS_LOG_ERR("%s:Invalid roi switch value:%d\n",
				__func__, info->roi_switch);
			ret = -EINVAL;
		}
		break;
	case TS_ACTION_READ:
	default:
		break;
	}
	TS_LOG_INFO("%s: roi_switch = %d\n", __func__, info->roi_switch);
	return ret;
}

static u8 *brl_roi_rawdata(void)
{
	u8 *rawdata_ptr = NULL;
	struct brl_ts_data *ts = brl_ts;
	struct ts_roi_info *roi_info = NULL;

	roi_info = &ts->dev_data->ts_platform_data->feature_info.roi_info;
	if (!roi_info->roi_supported) {
		TS_LOG_ERR("%s : Not supported roi mode\n", __func__);
		return NULL;
	}

	if (wait_for_completion_interruptible_timeout(&roi_data_done,
		msecs_to_jiffies(30))) {
		mutex_lock(&ts->roi.mutex);
		if (ts->roi.enabled && ts->roi.data_ready)
			rawdata_ptr = (u8 *)ts->roi.rawdata;
		else if (!ts->roi.data_ready && ts->roi.enabled)
			TS_LOG_ERR("%s:roi is not enable or data not ready\n",
				__func__);
		mutex_unlock(&ts->roi.mutex);
	} else {
		TS_LOG_ERR("%s : wait roi_data_done timeout!\n", __func__);
	}

	return rawdata_ptr;
}

#define BRL_ROI_BUF_MAX 50
static int brl_get_standard_matrix(struct roi_matrix *src_mat,
	struct roi_matrix *dst_mat)
{
	unsigned int i;
	unsigned int j;
	short temp_buf[BRL_ROI_BUF_MAX] = {0};
	short diff_max = 0x8000;
	unsigned int peak_row = 0;
	unsigned int peak_col = 0;
	unsigned int src_row_start;
	unsigned int src_row_end;
	unsigned int src_col_start;
	unsigned int src_col_end;
	unsigned int dst_row_start;
	unsigned int dst_col_start;

	/* transport src_mat matrix */
	for (i = 0; i < src_mat->row; i++) {
		for (j = 0; j < src_mat->col; j++)
			temp_buf[j * src_mat->col + i] =
				src_mat->data[i * src_mat->col + j];
	}

	i = src_mat->row;
	src_mat->row = src_mat->col;
	src_mat->col = i;
	memcpy(src_mat->data, temp_buf, src_mat->row *
		src_mat->col * sizeof(short));

	/* get peak value postion */
	for (i = 0; i < src_mat->row; i++) {
		for (j = 0; j < src_mat->col; j++) {
			if (src_mat->data[i * src_mat->col + j] > diff_max) {
				diff_max = src_mat->data[i * src_mat->col + j];
				peak_row = i;
				peak_col = j;
			}
		}
	}
	TS_LOG_DEBUG("Peak pos %d %d= %d\n",
		peak_row, peak_col, diff_max);
	src_row_start = 0;
	dst_row_start = ((dst_mat->row - 1) >> 1) - peak_row;
	if (peak_row >= ((dst_mat->row - 1) >> 1)) {
		src_row_start = peak_row - ((dst_mat->row - 1) >> 1);
		dst_row_start = 0;
	}

	src_row_end = src_mat->row - 1;
	if (peak_row <= (src_mat->row - 1 - (dst_mat->row >> 1)))
		src_row_end = peak_row + (dst_mat->row >> 1);

	src_col_start = 0;
	dst_col_start = ((dst_mat->col - 1) >> 1) - peak_col;
	if (peak_col >= ((dst_mat->col - 1) >> 1)) {
		src_col_start = peak_col - ((dst_mat->col - 1) >> 1);
		dst_col_start = 0;
	}

	src_col_end = src_mat->col - 1;
	if (peak_col <= (src_mat->col - 1 - (dst_mat->col >> 1)))
		src_col_end = peak_col + (dst_mat->col >> 1);

	/* copy peak value area to the center of ROI matrix */
	memset(dst_mat->data, 0, sizeof(short) * dst_mat->row * dst_mat->col);
	for (i = src_row_start; i <= src_row_end; i++)
		memcpy(&dst_mat->data[dst_col_start + dst_mat->col *
			(dst_row_start + i - src_row_start)],
			&src_mat->data[i * src_mat->col + src_col_start],
			(src_col_end - src_col_start + 1) * sizeof(short));

	return NO_ERR;
}

static int brl_cache_roidata_device(struct brl_ts_roi *roi)
{
	unsigned char status[ROI_HEAD_LEN] = {0};
	struct roi_matrix src_mat;
	struct roi_matrix dst_mat;
	u16 checksum = 0;
	unsigned int i;
	int res_write;
	int ret;
	struct brl_ts_data *ts = brl_ts;

	memset(&src_mat, 0, sizeof(src_mat));
	memset(&dst_mat, 0, sizeof(dst_mat));
	if (!roi || !roi->enabled)
		return -EINVAL;

	roi->data_ready = false;

	ret = brl_i2c_read(ts->brl_roi_data_add, status, ROI_HEAD_LEN);
	if (ret) {
		TS_LOG_ERR("Failed read ROI HEAD\n");
		return -EINVAL;
	}

	for (i = 0; i < ROI_HEAD_LEN; i++)
		checksum += status[i];

	if (unlikely((u8)checksum != 0)) {
		TS_LOG_ERR("roi status checksum error %02x %02x %02x %02x\n",
			status[0], status[1], status[2], status[3]);
		return -EINVAL;
	}

	if (status[0] & ROI_READY_MASK) {
		roi->track_id = status[0] & ROI_TRACKID_MASK;
	} else {
		TS_LOG_ERR("ROI data not ready\n");
		return -EINVAL;
	}

	dst_mat.row = roi->roi_rows;
	dst_mat.col = roi->roi_cols;
	src_mat.row = status[GTX8_ROI_SRC_STATUS_INDEX] & GTX8_BIT_MASK;
	src_mat.col = status[GTX8_ROI_SRC_STATUS_INDEX] >> 4;
	if (roi->roi_mat_src.data == NULL || roi->roi_mat_dst.data == NULL) {
		TS_LOG_ERR("roi_matrix is null\n");
		return -EINVAL;
	}
	src_mat.data = roi->roi_mat_src.data;
	dst_mat.data = roi->roi_mat_dst.data;

	/* read ROI rawdata */
	ret = brl_i2c_read((ts->brl_roi_data_add + ROI_HEAD_LEN),
		(char *)src_mat.data, (src_mat.row * src_mat.col + 1) *
		sizeof(src_mat.data[0]));
	if (ret) {
		TS_LOG_ERR("Failed read ROI rawdata\n");
		ret = -EINVAL;
		goto err_out;
	}

	for (i = 0, checksum = 0; i < src_mat.row * src_mat.col + 1; i++)
		checksum += src_mat.data[i];
	if (checksum) {
		TS_LOG_ERR("ROI rawdata checksum error\n");
		goto err_out;
	}

	if (brl_get_standard_matrix(&src_mat, &dst_mat)) {
		TS_LOG_ERR("Failed get standard matrix\n");
		goto err_out;
	}

	mutex_lock(&roi->mutex);
	memcpy(&roi->rawdata[0], &status[0], ROI_HEAD_LEN);
	memcpy(((char *)roi->rawdata) + ROI_HEAD_LEN, &dst_mat.data[0],
		   roi->roi_rows * roi->roi_cols * sizeof(*roi->rawdata));

	roi->data_ready = true;
	mutex_unlock(&roi->mutex);
	ret = 0;
err_out:

	/* clear the roi state flag */
	status[0] = 0x00;
	res_write = brl_i2c_write(ts->brl_roi_data_add, status, 1);
	if (res_write)
		TS_LOG_ERR("%s:clear ROI status failed %d\n", __func__,
			res_write);

	return ret;
}

static void brl_work_after_input_kit(void)
{
	struct brl_ts_data *ts = brl_ts;
	struct ts_roi_info *roi_info = NULL;

	roi_info = &ts->dev_data->ts_platform_data->feature_info.roi_info;

	if (roi_info->roi_supported) {
		/*
		 * We are about to refresh roi_data.
		 * To avoid stale output
		 * use a completion to block possible readers.
		 */
		reinit_completion(&roi_data_done);
		TS_LOG_DEBUG("%s: pre %d cur %d cur_index & pre_index:%d\n",
				__func__, pre_index, cur_index,
				(cur_index & pre_index));
		/* finger num id diff ,and must be finger done */
		if ((pre_index != cur_index) &&
			((cur_index & pre_index) != cur_index) &&
			(cur_finger_number > pre_finger_number))
			/* depend on driver to format data */
			brl_cache_roidata_device(&ts->roi);
		pre_index = cur_index;
		pre_finger_number = cur_finger_number;
		 /* If anyone has been blocked by us, wake it up */
		complete_all(&roi_data_done);
	}
}

static int brl_switch_gesture_mode(unsigned char double_click_cmd,
	unsigned char single_click_cmd)
{
	int retry = 0;
	struct brl_ts_cmd cmd;

	cmd.cmd = GOODIX_GESTURE_CMD;
	cmd.len = GESTURE_CMD_LEN;
	cmd.data[0] = double_click_cmd;
	cmd.data[1] = single_click_cmd;

	while (retry++ < GTX8_RETRY_NUM_3) {
		if (!brl_send_cmd(&cmd))
			return NO_ERR;
		TS_LOG_ERR("%s: send_cmd failed, retry = %d\n", __func__, retry);
	}
	TS_LOG_ERR("%s: Failed to switch gesture mode\n", __func__);
	return -EINVAL;
}

static int brl_chip_suspend(void)
{
	struct brl_ts_data *ts = brl_ts;
	int tskit_pt_station_flag = 0;
	int ret;
	struct lcd_kit_ops *tp_ops = lcd_kit_get_ops();
	struct ts_kit_platform_data *pdata = ts->dev_data->ts_platform_data;
	unsigned char double_click_cmd = 0;
	unsigned char single_click_cmd = 0;

	TS_LOG_INFO(" %s enter\n", __func__);
	ts->after_resume_done = false;

	if ((tp_ops) && (tp_ops->get_status_by_type)) {
		ret = tp_ops->get_status_by_type(PT_STATION_TYPE,
			&tskit_pt_station_flag);
		if (ret < 0) {
			TS_LOG_ERR("%s: get tskit_pt_station_flag failed\n",
				__func__);
			return ret;
		}
	}

	ts->suspend_status = true;
	usleep_range(10000, 11000); /* 10000:10000us  */

	switch ((int)(ts->dev_data->easy_wakeup_info.sleep_mode ||
		ts->dev_data->easy_wakeup_info.aod_mode)) {
	case TS_POWER_OFF_MODE:
		if (!tskit_pt_station_flag) {
			TS_LOG_INFO("%s: enter power_off mode\n", __func__);
			brl_power_switch(SWITCH_OFF);
		} else {
			TS_LOG_INFO("%s: enter sleep mode\n", __func__);
			brl_switch_workmode(SLEEP_MODE); /* sleep mode */
		}
		brl_pinctrl_select_suspend();
		break;
	case TS_GESTURE_MODE:
		TS_LOG_INFO("%s: enter gesture mode,aod:%d,gesture:%d\n",
			__func__, ts->dev_data->easy_wakeup_info.aod_mode,
			ts->dev_data->easy_wakeup_info.sleep_mode);
		if (!ts->dev_data->easy_wakeup_info.aod_mode)
			single_click_cmd = DISABLE_SINGLE_CLICK_CMD;
		if (!ts->dev_data->easy_wakeup_info.sleep_mode)
			double_click_cmd = DISABLE_DOUBLE_CLICK_CMD;
		brl_switch_gesture_mode(double_click_cmd, single_click_cmd);
		disable_irq(pdata->irq_id);
		brl_pinctrl_select_suspend();
		gpio_direction_output(pdata->i2c_switch_gpio, 1);
#ifdef CONFIG_HUAWEI_SHB_THP
		ret = send_thp_driver_status_cmd(POWER_OFF,
			pdata->multi_panel_index,
			ST_CMD_TYPE_MULTI_TP_UD_STATUS);
		if (ret < 0)
			TS_LOG_ERR("send cmd to sensorhub failed\n");
#endif
		TS_LOG_INFO("change to sensorhub i2c\n");
		break;
	default:
		TS_LOG_INFO("%s: default enter sleep mode\n", __func__);
		brl_switch_workmode(SLEEP_MODE);
		brl_pinctrl_select_suspend();
		break;
	}

	TS_LOG_INFO("%s: exit\n", __func__);
	return NO_ERR;
}

static int brl_chip_resume(void)
{
	struct brl_ts_data *ts = brl_ts;
	int tskit_pt_station_flag = 0;
	int ret;
	struct lcd_kit_ops *tp_ops = lcd_kit_get_ops();
	struct ts_kit_platform_data *pdata = ts->dev_data->ts_platform_data;

	TS_LOG_INFO("%s: enter\n", __func__);

	if ((tp_ops) && (tp_ops->get_status_by_type)) {
		ret = tp_ops->get_status_by_type(PT_STATION_TYPE,
			&tskit_pt_station_flag);
		if (ret < 0) {
			TS_LOG_ERR("%s: get tskit_pt_station_flag failed\n",
				__func__);
			return ret;
		}
	}

	switch ((int)(ts->dev_data->easy_wakeup_info.sleep_mode ||
		ts->dev_data->easy_wakeup_info.aod_mode)) {
	case TS_POWER_OFF_MODE:
		brl_pinctrl_select_normal();
		if (!tskit_pt_station_flag)
			brl_power_switch(SWITCH_ON);
		else
			brl_resume_chip_reset();
		break;
	case TS_GESTURE_MODE:
	default:
#ifdef CONFIG_HUAWEI_SHB_THP
		ret = send_thp_driver_status_cmd(POWER_ON,
			pdata->multi_panel_index,
			ST_CMD_TYPE_MULTI_TP_UD_STATUS);
		if (ret < 0)
			TS_LOG_ERR("%s: fail to send cmd to sensorhub\n",
			__func__);
#endif
		TS_LOG_INFO("change to ap i3c\n");
		/* delay 10 ms to wait for sensorhub deal with cmd */
		mdelay(10);
		gpio_direction_output(pdata->i2c_switch_gpio, 0);
		brl_pinctrl_select_normal();
		brl_resume_chip_reset();
		enable_irq(pdata->irq_id);
		break;
	}

	msleep(GTX8_RESET_SLEEP_TIME);
	TS_LOG_INFO(" %s:exit\n", __func__);
	return NO_ERR;
}

static int brl_chip_after_resume(void *feature_info)
{
	TS_LOG_INFO("%s: enter\n", __func__);

	if (brl_ts->rawdiff_mode_res == false)
		brl_ts->rawdiff_mode = false;
	brl_ts->suspend_status = false;
	brl_ts->after_resume_done = true;
	brl_status_resume();
	return NO_ERR;
}

static void brl_chip_shutdown(void)
{
	brl_power_switch(SWITCH_OFF);
}


struct ts_device_ops ts_brl_ops = {
	.chip_detect = brl_chip_detect,
	.chip_init = brl_chip_init,
	.chip_input_config = brl_input_config,
	.chip_irq_bottom_half = brl_irq_bottom_half,
	.chip_reset = brl_chip_reset,
	.chip_fw_update_boot = brl_fw_update_boot,
	.chip_fw_update_sd = brl_fw_update_sd,
	.chip_get_info = brl_chip_get_info,
	.chip_suspend = brl_chip_suspend,
	.chip_resume = brl_chip_resume,
	.chip_after_resume = brl_chip_after_resume,
	.chip_shutdown = brl_chip_shutdown,
	.chip_get_rawdata = brl_get_rawdata,
	.chip_glove_switch = brl_glove_switch,
	.chip_holster_switch = brl_holster_switch,
	.chip_charger_switch = brl_charger_switch,
	.chip_touch_switch = brl_touch_switch,
	.chip_roi_switch = brl_roi_switch,
	.chip_roi_rawdata = brl_roi_rawdata,
	.chip_work_after_input = brl_work_after_input_kit,
	.chip_get_capacitance_test_type =
		brl_chip_get_capacitance_test_type,
};

static int __init brl_ts_module_init(void)
{
	int ret = NO_ERR;
	bool found = false;
	struct device_node *child = NULL;
	struct device_node *root = NULL;

	TS_LOG_INFO("%s: called\n", __func__);
	brl_ts = kzalloc(sizeof(struct brl_ts_data), GFP_KERNEL);
	if (brl_ts == NULL) {
		TS_LOG_ERR("%s:alloc mem for device data failed\n", __func__);
		ret = -ENOMEM;
		return ret;
	}

	brl_ts->dev_data =
		kzalloc(sizeof(struct ts_kit_device_data), GFP_KERNEL);
	if (brl_ts->dev_data == NULL) {
		TS_LOG_ERR("%s:alloc mem for ts_kit_device data failed\n",
			__func__);
		ret = -ENOMEM;
		goto error_exit;
	}
	brl_ts->write_buf =
		kzalloc(GOODIX_TRANSFER_MAX_SIZE + 4, GFP_KERNEL);
	if (brl_ts->write_buf == NULL) {
		TS_LOG_ERR("%s:alloc mem for write_buf failed\n",
			__func__);
		ret = -ENOMEM;
		goto error_exit;
	}

	root = of_find_compatible_node(NULL, NULL, HUAWEI_TS_KIT);
	if (!root) {
		TS_LOG_ERR("%s:find_compatible_node error\n", __func__);
		ret = -EINVAL;
		goto error_exit;
	}

	for_each_child_of_node(root, child) {
		if (of_device_is_compatible(child, GTP_GTX8_CHIP_NAME)) {
			found = true;
			break;
		}
	}

	if (!found) {
		TS_LOG_ERR("%s:device tree node not found, name=%s\n",
			__func__, GTP_GTX8_CHIP_NAME);
		ret = -EINVAL;
		goto error_exit;
	}

	brl_ts->dev_data->cnode = child;
	brl_ts->dev_data->ops = &ts_brl_ops;
	ret = huawei_ts_chip_register(brl_ts->dev_data);
	if (ret) {
		TS_LOG_ERR("%s:chip register fail, ret=%d\n", __func__, ret);
		goto error_exit;
	}

	TS_LOG_INFO("%s:success\n", __func__);
	return ret;

error_exit:
	kfree(brl_ts->write_buf);
	brl_ts->write_buf = NULL;
	if (brl_ts->dev_data != NULL) {
		kfree(brl_ts->dev_data);
		brl_ts->dev_data = NULL;
	}
	if (brl_ts != NULL) {
		kfree(brl_ts);
		brl_ts = NULL;
	}

	TS_LOG_INFO("%s:failed\n", __func__);
	return ret;
}

static void __exit brl_ts_module_exit(void)
{
	kfree(brl_ts->roi.roi_mat_src.data);
	brl_ts->roi.roi_mat_src.data = NULL;
	kfree(brl_ts->roi.roi_mat_dst.data);
	brl_ts->roi.roi_mat_dst.data = NULL;
	kfree(brl_ts->roi.rawdata);
	brl_ts->roi.rawdata = NULL;

	kfree(brl_ts->write_buf);
	brl_ts->write_buf = NULL;
	kfree(brl_ts->dev_data);
	brl_ts->dev_data = NULL;

	kfree(brl_ts);
	brl_ts = NULL;
}

late_initcall(brl_ts_module_init);
module_exit(brl_ts_module_exit);
MODULE_AUTHOR("Huawei Device Company");
MODULE_DESCRIPTION("Huawei TouchScreen Driver");
MODULE_LICENSE("GPL");
