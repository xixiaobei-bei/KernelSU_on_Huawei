/*
 * stwlc68.c
 *
 * stwlc68 driver
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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
#include <chipset_common/hwpower/common_module/power_delay.h>

#define HWLOG_TAG wireless_stwlc68
HWLOG_REGIST();

static struct qi_protocol_handle *g_stwlc68_handle;
static struct stwlc68_dev_info *g_stwlc68_di;
static struct wakeup_source g_stwlc68_wakelock;
static int stop_charging_flag;
static int irq_abnormal_flag;
static bool g_tx_open_flag;

static const u8 stwlc68_rx2tx_header[STWLC68_SEND_MSG_PKT_LEN] = {
	0, 0x18, 0x28, 0x38, 0x48, 0x58
};
static const u8 stwlc68_tx2rx_header[STWLC68_RCVD_MSG_PKT_LEN] = {
	0, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f
};

static bool stwlc68_is_pwr_good(void)
{
	int gpio_val;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di)
		return false;
	if (di->gpio_pwr_good <= 0)
		return true;
	if (power_cmdline_is_factory_mode())
		return true;

	gpio_val = gpio_get_value(di->gpio_pwr_good);

	return gpio_val == STWLC68_GPIO_PWR_GOOD_VAL;
}

static bool stwlc68_is_i2c_addr_valid(struct stwlc68_dev_info *di, u16 addr)
{
	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return false;
	}

	if (!di->g_val.sram_chk_complete) {
		if ((addr < STWLC68_RAM_FW_START_ADDR + STWLC68_RAM_MAX_SIZE) &&
			(addr >= STWLC68_RAM_FW_START_ADDR))
			return true;

		return false;
	}

	return true;
}

static int stwlc68_i2c_read(struct i2c_client *client, u8 *cmd, int cmd_len,
	u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!stwlc68_is_pwr_good())
			return -WLC_ERR_I2C_R;
		if (!power_i2c_read_block(client, cmd, cmd_len, buf, buf_len))
			return 0;
		usleep_range(9500, 10500); /* 10ms */
	}

	return -WLC_ERR_I2C_R;
}

static int stwlc68_i2c_write(struct i2c_client *client, u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!stwlc68_is_pwr_good())
			return -WLC_ERR_I2C_W;
		if (!power_i2c_write_block(client, buf, buf_len))
			return 0;
		usleep_range(9500, 10500); /* 10ms */
	}

	return -WLC_ERR_I2C_W;
}

int stwlc68_read_block(struct stwlc68_dev_info *di, u16 reg, u8 *data, u8 len)
{
	u8 cmd[STWLC68_ADDR_LEN];

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (!stwlc68_is_i2c_addr_valid(di, reg))
		return -WLC_ERR_I2C_R;

	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;

	return stwlc68_i2c_read(di->client, cmd, STWLC68_ADDR_LEN, data, len);
}

int stwlc68_write_block(struct stwlc68_dev_info *di, u16 reg, u8 *data, u8 data_len)
{
	int ret;
	u8 *cmd = NULL;

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	if (!stwlc68_is_i2c_addr_valid(di, reg))
		return -WLC_ERR_I2C_W;

	cmd = kzalloc(sizeof(u8) * (STWLC68_ADDR_LEN + data_len), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	memcpy(&cmd[STWLC68_ADDR_LEN], data, data_len);

	ret = stwlc68_i2c_write(di->client, cmd, STWLC68_ADDR_LEN + data_len);

	kfree(cmd);
	return ret;
}

int stwlc68_4addr_read_block(struct stwlc68_dev_info *di, u32 addr, u8 *data, u8 len)
{
	u8 cmd[STWLC68_4ADDR_F_LEN];

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	 /* bit[0]: flag 0xFA; bit[1:4]: addr */
	cmd[0] = STWLC68_4ADDR_FLAG;
	cmd[1] = (u8)((addr >> 24) & BYTE_MASK);
	cmd[2] = (u8)((addr >> 16) & BYTE_MASK);
	cmd[3] = (u8)((addr >> 8) & BYTE_MASK);
	cmd[4] = (u8)((addr >> 0) & BYTE_MASK);

	return stwlc68_i2c_read(di->client,
		cmd, STWLC68_4ADDR_F_LEN, data, len);
}

int stwlc68_4addr_write_block(struct stwlc68_dev_info *di, u32 addr, u8 *data, u8 data_len)
{
	int ret;
	u8 *cmd = NULL;

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	cmd = kzalloc(sizeof(u8) * (STWLC68_4ADDR_F_LEN + data_len), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	 /* bit[0]: flag 0xFA; bit[1:4]: addr */
	cmd[0] = STWLC68_4ADDR_FLAG;
	cmd[1] = (u8)((addr >> 24) & BYTE_MASK);
	cmd[2] = (u8)((addr >> 16) & BYTE_MASK);
	cmd[3] = (u8)((addr >> 8) & BYTE_MASK);
	cmd[4] = (u8)((addr >> 0) & BYTE_MASK);
	memcpy(&cmd[STWLC68_4ADDR_F_LEN], data, data_len);

	ret = stwlc68_i2c_write(di->client, cmd, STWLC68_4ADDR_F_LEN + data_len);

	kfree(cmd);
	return ret;
}

int stwlc68_read_byte(u16 reg, u8 *data)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	return stwlc68_read_block(di, reg, data, BYTE_LEN);
}

int stwlc68_write_byte(u16 reg, u8 data)
{
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	return stwlc68_write_block(di, reg, &data, BYTE_LEN);
}

int stwlc68_read_word(u16 reg, u16 *data)
{
	int ret;
	u8 buff[WORD_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !data) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_block(di, reg, buff, WORD_LEN);
	if (ret)
		return -WLC_ERR_I2C_R;

	*data = buff[0] | buff[1] << BITS_PER_BYTE;
	return 0;
}

int stwlc68_write_word(u16 reg, u16 data)
{
	u8 buff[WORD_LEN];
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return stwlc68_write_block(di, reg, buff, WORD_LEN);
}

static int stwlc68_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data)
{
	int ret;
	u16 val = 0;

	ret = stwlc68_read_word(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return stwlc68_write_word(reg, val);
}

static int stwlc68_clear_interrupt(u16 intr)
{
	int ret;

	ret = stwlc68_write_word(STWLC68_INTR_CLR_ADDR, intr);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static void stwlc68_wake_lock(void)
{
	if (!g_stwlc68_wakelock.active) {
		__pm_stay_awake(&g_stwlc68_wakelock);
		hwlog_info("%s\n", __func__);
	}
}

static void stwlc68_wake_unlock(void)
{
	if (g_stwlc68_wakelock.active) {
		__pm_relax(&g_stwlc68_wakelock);
		hwlog_info("%s\n", __func__);
	}
}

void stwlc68_enable_irq(struct stwlc68_dev_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (!di->irq_active) {
		hwlog_info("[%s] ++\n", __func__);
		enable_irq(di->irq_int);
		di->irq_active = 1;
	}
	hwlog_info("[%s] --\n", __func__);
	mutex_unlock(&di->mutex_irq);
}

void stwlc68_disable_irq_nosync(struct stwlc68_dev_info *di)
{
	mutex_lock(&di->mutex_irq);
	if (di->irq_active) {
		hwlog_info("[%s] ++\n", __func__);
		disable_irq_nosync(di->irq_int);
		di->irq_active = 0;
	}
	hwlog_info("[%s] --\n", __func__);
	mutex_unlock(&di->mutex_irq);
}

void stwlc68_chip_enable(bool enable, void *dev_data)
{
	int gpio_val;
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return;

	gpio_set_value(di->gpio_en,
		enable ? di->gpio_en_valid_val : !di->gpio_en_valid_val);
	gpio_val = gpio_get_value(di->gpio_en);
	hwlog_info("[%s] gpio is %s now\n",
		__func__, gpio_val ? "high" : "low");
}

static bool stwlc68_is_chip_enable(void *dev_data)
{
	int gpio_val;
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return false;

	gpio_val = gpio_get_value(di->gpio_en);
	return gpio_val == di->gpio_en_valid_val;
}

static void stwlc68_sleep_enable(bool enable, void *dev_data)
{
	int gpio_val;
	struct stwlc68_dev_info *di = dev_data;

	if (!di || irq_abnormal_flag)
		return;

	gpio_set_value(di->gpio_sleep_en,
		enable ? WLTRX_IC_SLEEP_ENABLE : WLTRX_IC_SLEEP_DISABLE);
	gpio_val = gpio_get_value(di->gpio_sleep_en);
	hwlog_info("[%s] gpio is %s now\n",
		__func__, gpio_val ? "high" : "low");
}

static bool stwlc68_is_sleep_enable(void *dev_data)
{
	int gpio_val;
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return false;

	gpio_val = gpio_get_value(di->gpio_sleep_en);
	return gpio_val == WLTRX_IC_SLEEP_ENABLE;
}

static void stwlc68_chip_reset(void *dev_data)
{
	int ret;
	u8 data = STWLC68_CHIP_RESET;
	struct stwlc68_dev_info *di = dev_data;

	ret = stwlc68_4addr_write_block(di, STWLC68_CHIP_RESET_ADDR, &data,
		STWLC68_FW_ADDR_LEN);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return;
	}

	hwlog_info("[%s] succ\n", __func__);
}

static void stwlc68_ext_pwr_prev_ctrl(bool flag, void *dev_data)
{
	int ret;
	u8 wr_buff;
	struct stwlc68_dev_info *di = dev_data;

	if (flag)
		wr_buff = STWLC68_LDO5V_EN;
	else
		wr_buff = STWLC68_LDO5V_DIS;

	hwlog_info("[%s] ldo_5v %s\n", __func__, flag ? "on" : "off");
	ret = stwlc68_4addr_write_block(di, STWLC68_LDO5V_EN_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		hwlog_err("ext_pwr_prev_ctrl: write reg failed\n");
}

static int stwlc68_get_chip_id(u16 *chip_id)
{
	return stwlc68_read_word(STWLC68_CHIP_ID_ADDR, chip_id);
}

int stwlc68_get_cfg_id(u16 *cfg_id)
{
	return stwlc68_read_word(STWLC68_CFG_ID_ADDR, cfg_id);
}

int stwlc68_get_patch_id(u16 *patch_id)
{
	return stwlc68_read_word(STWLC68_PATCH_ID_ADDR, patch_id);
}

int stwlc68_get_cut_id(struct stwlc68_dev_info *di, u8 *cut_id)
{
	if (!di)
		return -ENODEV;

	return stwlc68_4addr_read_block(di, STWLC68_CUT_ID_ADDR,
		cut_id, STWLC68_FW_ADDR_LEN);
}

int stwlc68_get_chip_info(struct stwlc68_chip_info *info)
{
	int ret;
	u8 chip_info[STWLC68_CHIP_INFO_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_block(di, STWLC68_CHIP_INFO_ADDR,
		chip_info, STWLC68_CHIP_INFO_LEN);
	if (ret)
		return ret;

	/*
	 * addr[0:1]:     chip unique id;
	 * addr[2:2]:     chip revision number;
	 * addr[3:3]:     customer id;
	 * addr[4:5]:     sram id;
	 * addr[6:7]:     svn revision number;
	 * addr[8:9]:     configuration id;
	 * addr[10:11]:  pe id;
	 * addr[12:13]:  patch id;
	 * 1byte = 8bit
	 */
	info->chip_id = (u16)(chip_info[0] | (chip_info[1] << 8));
	info->chip_rev = chip_info[2];
	info->cust_id = chip_info[3];
	info->sram_id = (u16)(chip_info[4] | (chip_info[5] << 8));
	info->svn_rev = (u16)(chip_info[6] | (chip_info[7] << 8));
	info->cfg_id = (u16)(chip_info[8] | (chip_info[9] << 8));
	info->pe_id = (u16)(chip_info[10] | (chip_info[11] << 8));
	info->patch_id = (u16)(chip_info[12] | (chip_info[13] << 8));

	ret = stwlc68_get_cut_id(di, &info->cut_id);
	if (ret)
		return ret;

	return 0;
}

int stwlc68_get_chip_info_str(char *info_str, int len, void *dev_data)
{
	int ret;
	struct stwlc68_chip_info chip_info = { 0 };

	if (!info_str || (len < WLTRX_IC_CHIP_INFO_LEN))
		return -EINVAL;

	ret = stwlc68_get_chip_info(&chip_info);
	if (ret)
		return -EIO;

	return snprintf(info_str, WLTRX_IC_CHIP_INFO_LEN,
		"chip_id:%d cfg_id:0x%x patch_id:0x%x cut_id:%d sram_id:0x%x",
		chip_info.chip_id, chip_info.cfg_id, chip_info.patch_id,
		chip_info.cut_id, chip_info.sram_id);
}

static int stwlc68_get_mode(u8 *mode)
{
	int ret;

	if (!mode)
		return WLC_ERR_PARA_NULL;

	ret = stwlc68_read_byte(STWLC68_OP_MODE_ADDR, mode);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return 0;
}

static int stwlc68_get_rx_temp(int *temp, void *dev_data)
{
	s16 l_temp = 0;

	if (!temp || stwlc68_read_word(STWLC68_CHIP_TEMP_ADDR, (u16 *)&l_temp))
		return -EINVAL;

	*temp = l_temp;
	return 0;
}

static int stwlc68_get_rx_fop(int *fop, void *dev_data)
{
	return stwlc68_read_word(STWLC68_OP_FREQ_ADDR, (u16 *)fop);
}

static int stwlc68_get_rx_cep(int *cep, void *dev_data)
{
	return 0;
}

static int stwlc68_get_rx_vrect(int *vrect, void *dev_data)
{
	return stwlc68_read_word(STWLC68_VRECT_ADDR, (u16 *)vrect);
}

static int stwlc68_get_rx_vout(int *vout, void *dev_data)
{
	return stwlc68_read_word(STWLC68_VOUT_ADDR, (u16 *)vout);
}

static int stwlc68_get_rx_iout(int *iout, void *dev_data)
{
	return stwlc68_read_word(STWLC68_IOUT_ADDR, (u16 *)iout);
}

static int stwlc68_get_rx_vout_reg(int *vreg, void *dev_data)
{
	int ret;

	ret = stwlc68_read_word(STWLC68_RX_VOUT_SET_ADDR, (u16 *)vreg);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	*vreg *= STWLC68_RX_VOUT_SET_STEP;
	return 0;
}

static int stwlc68_get_vfc_reg(int *vfc_reg, void *dev_data)
{
	return stwlc68_read_word(STWLC68_FC_VOLT_ADDR, (u16 *)vfc_reg);
}

static int stwlc68_set_rx_vout(int vol, void *dev_data)
{
	int ret;

	if (vol < STWLC68_RX_VOUT_MIN || vol > STWLC68_RX_VOUT_MAX) {
		hwlog_err("%s: out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	vol = vol / STWLC68_RX_VOUT_SET_STEP;
	ret = stwlc68_write_word(STWLC68_RX_VOUT_SET_ADDR, (u16)vol);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	return 0;
}

static bool stwlc68_is_cp_really_open(void *dev_data)
{
	int rx_ratio;
	int rx_vset = 0;
	int rx_vout = 0;
	int cp_vout;

	if (!charge_pump_is_cp_open(CP_TYPE_MAIN))
		return false;

	rx_ratio = charge_pump_get_cp_ratio(CP_TYPE_MAIN);
	(void)stwlc68_get_rx_vout_reg(&rx_vset, dev_data);
	(void)stwlc68_get_rx_vout(&rx_vout, dev_data);
	cp_vout = charge_pump_get_cp_vout(CP_TYPE_MAIN);
	cp_vout = (cp_vout > 0) ? cp_vout : wldc_get_ls_vbus();

	hwlog_info("[%s] rx_ratio:%d rx_vset:%d rx_vout:%d cp_vout:%d\n",
		__func__, rx_ratio, rx_vset, rx_vout, cp_vout);
	if ((cp_vout * rx_ratio) < (rx_vout - STWLC68_FC_VOUT_ERR_LTH))
		return false;
	if ((cp_vout * rx_ratio) > (rx_vout + STWLC68_FC_VOUT_ERR_UTH))
		return false;

	return true;
}

static int stwlc68_check_cp_mode(void *dev_data)
{
	int i;
	int cnt;
	int ret;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di)
		return -WLC_ERR_PARA_NULL;

	if (!di->support_cp)
		return 0;

	ret = charge_pump_set_cp_mode(CP_TYPE_MAIN);
	if (ret) {
		hwlog_err("%s: set cp mode fail\n", __func__);
		return ret;
	}
	cnt = STWLC68_BPCP_TIMEOUT / STWLC68_BPCP_SLEEP_TIME;
	for (i = 0; i < cnt; i++) {
		msleep(STWLC68_BPCP_SLEEP_TIME);
		if (stwlc68_is_cp_really_open(dev_data)) {
			hwlog_info("[%s] set cp mode succ\n", __func__);
			break;
		}
		if (stop_charging_flag)
			return -WLC_ERR_STOP_CHRG;
	}
	if (i == cnt) {
		hwlog_err("%s: set cp mode fail\n", __func__);
		return -WLC_ERR_MISMATCH;
	}

	return 0;
}

static int stwlc68_send_fc_cmd(int vset, void *dev_data)
{
	int ret;

	ret = stwlc68_write_word(STWLC68_FC_VOLT_ADDR, (u16)vset);
	if (ret) {
		hwlog_err("%s: set fc reg fail\n", __func__);
		return ret;
	}
	ret = stwlc68_write_word_mask(STWLC68_CMD_ADDR,
		STWLC68_CMD_SEND_FC, STWLC68_CMD_SEND_FC_SHIFT,
		STWLC68_CMD_VAL);
	if (ret) {
		hwlog_err("%s: send fc cmd fail\n", __func__);
		return ret;
	}
	ret = stwlc68_set_rx_vout(vset, dev_data);
	if (ret) {
		hwlog_err("%s: set rx vout fail\n", __func__);
		return ret;
	}

	return 0;
}

static bool stwlc68_is_fc_succ(int vset, void *dev_data)
{
	int i;
	int cnt;
	int vout = 0;

	cnt = STWLC68_FC_VOUT_TIMEOUT / STWLC68_FC_VOUT_SLEEP_TIME;
	for (i = 0; i < cnt; i++) {
		if (stop_charging_flag && (vset > STWLC68_FC_VOUT_DEFAULT))
			return false;
		msleep(STWLC68_FC_VOUT_SLEEP_TIME);
		(void)stwlc68_get_rx_vout(&vout, dev_data);
		if ((vout >= vset - STWLC68_FC_VOUT_ERR_LTH) &&
			(vout <= vset + STWLC68_FC_VOUT_ERR_UTH)) {
			hwlog_info("[%s] succ, cost_time: %dms\n", __func__,
				(i + 1) * STWLC68_FC_VOUT_SLEEP_TIME);
			return true;
		}
	}

	return false;
}

static void stwlc68_ask_mode_cfg(u8 mode_cfg)
{
	int ret;
	u8 val = 0;

	ret = stwlc68_write_byte(STWLC68_ASK_CFG_ADDR, mode_cfg);
	if (ret)
		hwlog_err("ask_mode_cfg: write fail\n");

	ret = stwlc68_read_byte(STWLC68_ASK_CFG_ADDR, &val);
	if (ret) {
		hwlog_err("ask_mode_cfg: read fail\n");
		return;
	}

	hwlog_info("[ask_mode_cfg] val=0x%x\n", val);
}

static void stwlc68_set_mode_cfg(int vset)
{
	if (vset <= RX_HIGH_VOUT) {
		stwlc68_ask_mode_cfg(STWLC68_BOTH_CAP_POSITIVE);
	} else {
		if (!power_cmdline_is_factory_mode())
			stwlc68_ask_mode_cfg(STWLC68_MOD_C_NEGATIVE);
		else
			stwlc68_ask_mode_cfg(STWLC68_BOTH_CAP_POSITIVE);
	}
}

static int stwlc68_set_vfc(int vfc, void *dev_data)
{
	int ret;
	int i;

	if (vfc >= RX_HIGH_VOUT2) {
		ret = stwlc68_check_cp_mode(dev_data);
		if (ret)
			return ret;
	}
	stwlc68_set_mode_cfg(vfc);

	for (i = 0; i < STWLC68_FC_VOUT_RETRY_CNT; i++) {
		if (stop_charging_flag && (vfc > STWLC68_FC_VOUT_DEFAULT))
			return -WLC_ERR_STOP_CHRG;
		ret = stwlc68_send_fc_cmd(vfc, dev_data);
		if (ret) {
			hwlog_err("%s: send fc_cmd fail\n", __func__);
			continue;
		}
		hwlog_info("[%s] send fc cmd, cnt: %d\n", __func__, i);
		if (stwlc68_is_fc_succ(vfc, dev_data)) {
			if (vfc < RX_HIGH_VOUT2)
				(void)charge_pump_set_bp_mode(CP_TYPE_MAIN);
			stwlc68_set_mode_cfg(vfc);
			hwlog_info("[%s] succ\n", __func__);
			return 0;
		}
	}

	return -WLC_ERR_MISMATCH;
}


static int stwlc68_send_msg(u8 cmd, u8 *data, int data_len, void *dev_data)
{
	int ret;
	u8 header;
	u8 write_data[STWLC68_SEND_MSG_DATA_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_I2C_WR;
	}

	if ((data_len > STWLC68_SEND_MSG_DATA_LEN) || (data_len < 0)) {
		hwlog_err("%s: send data number out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	di->irq_val &= ~STWLC68_RX_STATUS_TX2RX_ACK;
	/* msg_len=cmd_len+data_len  cmd_len=1 */
	header = stwlc68_rx2tx_header[data_len + 1];

	ret = stwlc68_write_byte(STWLC68_SEND_MSG_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("%s: write header fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}
	ret = stwlc68_write_byte(STWLC68_SEND_MSG_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("%s: write cmd fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	if (data && (data_len > 0)) {
		memcpy(write_data, data, data_len);
		ret = stwlc68_write_block(di,
			STWLC68_SEND_MSG_DATA_ADDR, write_data, data_len);
		if (ret) {
			hwlog_err("%s: write RX2TX-reg fail\n", __func__);
			return -WLC_ERR_I2C_W;
		}
	}

	ret = stwlc68_write_word_mask(STWLC68_CMD_ADDR,
		STWLC68_CMD_SEND_MSG_WAIT_RPLY,
		STWLC68_CMD_SEND_MSG_WAIT_RPLY_SHIFT, STWLC68_CMD_VAL);
	if (ret) {
		hwlog_err("%s: send RX msg to TX fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	hwlog_info("%s: send msg(cmd:0x%x) success\n", __func__, cmd);
	return 0;
}

static int stwlc68_send_msg_ack(u8 cmd, u8 *data, int data_len, void *dev_data)
{
	int ret;
	int count = 0;
	int ack_cnt;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	do {
		(void)stwlc68_send_msg(cmd, data, data_len, di);
		for (ack_cnt = 0;
			ack_cnt < STWLC68_WAIT_FOR_ACK_RETRY_CNT; ack_cnt++) {
			msleep(STWLC68_WAIT_FOR_ACK_SLEEP_TIME);
			if (STWLC68_RX_STATUS_TX2RX_ACK & di->irq_val) {
				di->irq_val &= ~STWLC68_RX_STATUS_TX2RX_ACK;
				hwlog_info("[%s] succ, retry times = %d\n",
					   __func__, count);
				return 0;
			}
			if (stop_charging_flag)
				return -WLC_ERR_STOP_CHRG;
		}
		count++;
		hwlog_info("[%s] retry\n", __func__);
	} while (count < STWLC68_SNED_MSG_RETRY_CNT);

	if (count < STWLC68_SNED_MSG_RETRY_CNT) {
		hwlog_info("[%s] succ\n", __func__);
		return 0;
	}

	ret = stwlc68_read_byte(STWLC68_RCVD_MSG_CMD_ADDR, &cmd);
	if (ret) {
		hwlog_err("%s: get rcv cmd data fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}
	if (cmd != STWLC68_CMD_ACK) {
		hwlog_err("[%s] fail, ack = 0x%x, retry times = %d\n",
			__func__, cmd, count);
		return -WLC_ERR_ACK_TIMEOUT;
	}

	return 0;
}

static int stwlc68_receive_msg(u8 *data, int data_len, void *dev_data)
{
	int ret;
	int count = 0;
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !data) {
		hwlog_err("%s: para null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	do {
		if (di->irq_val & STWLC68_RX_RCVD_MSG_INTR_LATCH) {
			di->irq_val &= ~STWLC68_RX_RCVD_MSG_INTR_LATCH;
			goto func_end;
		}
		if (stop_charging_flag)
			return -WLC_ERR_STOP_CHRG;
		msleep(STWLC68_RCV_MSG_SLEEP_TIME);
		count++;
	} while (count < STWLC68_RCV_MSG_SLEEP_CNT);

func_end:
	ret = stwlc68_read_block(di, STWLC68_RCVD_MSG_CMD_ADDR, data, data_len);
	if (ret) {
		hwlog_err("%s:get tx to rx data fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}
	if (!data[0]) { /* data[0]: cmd */
		hwlog_err("%s: no msg received from tx\n", __func__);
		return -WLC_ERR_ACK_TIMEOUT;
	}
	hwlog_info("[%s] get tx2rx data(cmd:0x%x) succ\n", __func__, data[0]);
	return 0;
}

static void stwlc68_send_ept(int ept_type, void *dev_data)
{
	int ret;
	u8 rx_ept_type;

	switch (ept_type) {
	case WIRELESS_EPT_ERR_VRECT:
		rx_ept_type = STWLC68_EPT_ERR_VRECT;
		break;
	case WIRELESS_EPT_ERR_VOUT:
		rx_ept_type = STWLC68_EPT_ERR_VOUT;
		break;
	default:
		return;
	}
	ret = stwlc68_write_byte(STWLC68_EPT_MSG_ADDR, rx_ept_type);
	ret += stwlc68_write_word_mask(STWLC68_CMD_ADDR, STWLC68_CMD_SEND_EPT,
		STWLC68_CMD_SEND_EPT_SHIFT, STWLC68_CMD_VAL);
	if (ret)
		hwlog_err("send_ept: failed\n");
}

int stwlc68_sw2tx(void)
{
	int ret;
	int i;
	u8 mode = 0;
	int cnt = STWLC68_SW2TX_RETRY_TIME / STWLC68_SW2TX_RETRY_SLEEP_TIME;

	for (i = 0; i < cnt; i++) {
		if (!g_tx_open_flag) {
			hwlog_err("%s: tx_open_flag false\n", __func__);
			return -WLC_ERR_STOP_CHRG;
		}
		msleep(STWLC68_SW2TX_RETRY_SLEEP_TIME);
		ret = stwlc68_get_mode(&mode);
		if (ret) {
			hwlog_err("%s: get mode fail\n", __func__);
			continue;
		}

		ret = stwlc68_write_word_mask(STWLC68_CMD_ADDR,
			STWLC68_CMD_SW2TX, STWLC68_CMD_SW2TX_SHIFT,
			STWLC68_CMD_VAL);
		if (ret) {
			hwlog_err("%s: write cmd(sw2tx) fail\n", __func__);
			continue;
		}
		if (mode == STWLC68_TX_MODE) {
			hwlog_info("%s: succ, cnt = %d\n", __func__, i);
			return 0;
		}
	}
	hwlog_err("%s: fail, cnt = %d\n", __func__, i);
	return -WLC_ERR_I2C_WR;
}

static int stwlc68_rx_fw_update(void *dev_data)
{
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return stwlc68_fw_sram_update(di, WIRELESS_RX);
}

static int stwlc68_tx_fw_update(void *dev_data)
{
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	return stwlc68_fw_sram_update(di, WIRELESS_TX);
}

static bool stwlc68_need_check_pu_shell(void *dev_data)
{
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return false;

	return di->need_chk_pu_shell;
}

static void stwlc68_set_pu_shell_flag(bool flag, void *dev_data)
{
	struct stwlc68_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}

	di->pu_shell_flag = flag;
	hwlog_info("[%s] %s succ\n", __func__, flag ? "true" : "false");
}

static bool stwlc68_is_tx_exist(void *dev_data)
{
	int ret;
	u8 mode = 0;

	ret = stwlc68_get_mode(&mode);
	if (ret) {
		hwlog_err("%s: get rx mode fail\n", __func__);
		return false;
	}

	return mode == STWLC68_RX_MODE;
}

static bool stwlc68_in_rx_mode(void *dev_data)
{
	return stwlc68_is_tx_exist(dev_data);
}

static int stwlc68_kick_watchdog(void *dev_data)
{
	int ret;

	ret = stwlc68_write_byte(STWLC68_WDT_FEED_ADDR, 0);
	if (ret)
		return -WLC_ERR_I2C_W;

	return 0;
}

static int stwlc68_get_rx_fod(char *fod_str, int len, void *dev_data)
{
	int i;
	int ret;
	char tmp[STWLC68_RX_FOD_TMP_STR_LEN] = { 0 };
	u8 fod_arr[STWLC68_RX_FOD_LEN] = { 0 };
	struct stwlc68_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -EINVAL;
	}

	if (!fod_str || (len < WLRX_IC_FOD_COEF_LEN))
		return -EINVAL;

	ret = stwlc68_read_block(di, STWLC68_RX_FOD_ADDR,
		fod_arr, STWLC68_RX_FOD_LEN);
	if (ret) {
		hwlog_err("%s: read fod fail\n", __func__);
		return -EIO;
	}

	for (i = 0; i < STWLC68_RX_FOD_LEN; i++) {
		snprintf(tmp, STWLC68_RX_FOD_TMP_STR_LEN, "%x ", fod_arr[i]);
		strncat(fod_str, tmp, strlen(tmp));
	}

	return strlen(fod_str);
}

static int stwlc68_set_rx_fod(const char *fod_str, void *dev_data)
{
	char *cur = (char *)fod_str;
	char *token = NULL;
	int i;
	u8 val = 0;
	const char *sep = " ,";
	u8 fod_arr[STWLC68_RX_FOD_LEN] = { 0 };
	struct stwlc68_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}
	if (!fod_str) {
		hwlog_err("%s: input fod_str err\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	for (i = 0; i < STWLC68_RX_FOD_LEN; i++) {
		token = strsep(&cur, sep);
		if (!token) {
			hwlog_err("%s: input fod_str number err\n", __func__);
			return -WLC_ERR_PARA_WRONG;
		}
		if (kstrtou8(token, POWER_BASE_DEC, &val)) {
			hwlog_err("%s: input fod_str type err\n", __func__);
			return -WLC_ERR_PARA_WRONG;
		}
		fod_arr[i] = val;
		hwlog_info("[%s] fod[%d] = 0x%x\n", __func__, i, fod_arr[i]);
	}

	return stwlc68_write_block(di, STWLC68_RX_FOD_ADDR,
		fod_arr, STWLC68_RX_FOD_LEN);
}

static int stwlc68_init_rx_fod_coef(struct stwlc68_dev_info *di, int tx_type)
{
	int vfc_reg = 0;
	int ret;
	u8 rx_fod[STWLC68_RX_FOD_LEN] = { 0 };
	u8 rx_offset = 0;

	(void)stwlc68_get_vfc_reg(&vfc_reg, di);
	hwlog_info("[%s] vfc_reg: %dmV\n", __func__, vfc_reg);

	if (vfc_reg < 9000) { /* (0, 9)V, set 5v fod */
		if (di->pu_shell_flag && di->need_chk_pu_shell)
			memcpy(rx_fod, di->pu_rx_fod_5v, sizeof(rx_fod));
		else
			memcpy(rx_fod, di->rx_fod_5v, sizeof(rx_fod));
	} else if (vfc_reg < 12000) { /* [9, 12)V, set 9V fod */
		if (tx_type == WIRELESS_TX_TYPE_CP60 ||
			tx_type == WIRELESS_TX_TYPE_CP85) {
			memcpy(rx_fod, di->rx_fod_9v_cp60, sizeof(rx_fod));
		} else if (tx_type == WIRELESS_TX_TYPE_CP39S) {
			memcpy(rx_fod, di->rx_fod_9v_cp39s, sizeof(rx_fod));
		} else {
			rx_offset = di->rx_offset_9v;
			memcpy(rx_fod, di->rx_fod_9v, sizeof(rx_fod));
		}
	} else if (vfc_reg < 15000) { /* [12, 15)V, set 12V fod */
		memcpy(rx_fod, di->rx_fod_12v, sizeof(rx_fod));
	} else if (vfc_reg < 18000) { /* [15, 18)V, set 15V fod */
		if (tx_type == WIRELESS_TX_TYPE_CP39S) {
			memcpy(rx_fod, di->rx_fod_15v_cp39s, sizeof(rx_fod));
		} else {
			rx_offset = di->rx_offset_15v;
			memcpy(rx_fod, di->rx_fod_15v, sizeof(rx_fod));
		}
	}

	ret = stwlc68_write_block(di, STWLC68_RX_FOD_ADDR,
		rx_fod, STWLC68_RX_FOD_LEN);
	if (ret) {
		hwlog_err("%s: write fod fail\n", __func__);
		return ret;
	}

	ret = stwlc68_write_byte(STWLC68_RX_OFFSET_ADDR, rx_offset);
	if (ret)
		hwlog_err("%s: write offset fail\n", __func__);

	return ret;
}

static int stwlc68_chip_init(unsigned int init_type, unsigned int tx_type, void *dev_data)
{
	int ret = 0;
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	switch (init_type) {
	case WIRELESS_CHIP_INIT:
		hwlog_info("[%s] DEFAULT_CHIP_INIT\n", __func__);
		stop_charging_flag = 0;
		ret += stwlc68_write_byte(STWLC68_FC_VRECT_DELTA_ADDR,
			STWLC68_FC_VRECT_DELTA / STWLC68_FC_VRECT_DELTA_STEP);
		/* fall through */
	case ADAPTER_5V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 5V_CHIP_INIT\n", __func__);
		ret += stwlc68_write_block(di, STWLC68_LDO_CFG_ADDR,
			di->rx_ldo_cfg_5v, STWLC68_LDO_CFG_LEN);
		ret += stwlc68_init_rx_fod_coef(di, tx_type);
		stwlc68_set_mode_cfg(init_type);
		break;
	case ADAPTER_9V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 9V_CHIP_INIT\n", __func__);
		ret += stwlc68_write_block(di, STWLC68_LDO_CFG_ADDR,
			di->rx_ldo_cfg_9v, STWLC68_LDO_CFG_LEN);
		ret += stwlc68_init_rx_fod_coef(di, tx_type);
		break;
	case ADAPTER_12V * WL_MVOLT_PER_VOLT:
		hwlog_info("[%s] 12V_CHIP_INIT\n", __func__);
		ret += stwlc68_write_block(di, STWLC68_LDO_CFG_ADDR,
			di->rx_ldo_cfg_12v, STWLC68_LDO_CFG_LEN);
		ret += stwlc68_init_rx_fod_coef(di, tx_type);
		break;
	case WILREESS_SC_CHIP_INIT:
		hwlog_info("[%s] SC_CHIP_INIT\n", __func__);
		ret += stwlc68_write_block(di, STWLC68_LDO_CFG_ADDR,
			di->rx_ldo_cfg_sc, STWLC68_LDO_CFG_LEN);
		ret += stwlc68_init_rx_fod_coef(di, tx_type);
		break;
	default:
		hwlog_info("%s: input para is invalid\n", __func__);
		break;
	}

	return ret;
}

static void stwlc68_stop_charging(void *dev_data)
{
	struct stwlc68_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("%s: para null\n", __func__);
		return;
	}

	stop_charging_flag = 1;
	if (!irq_abnormal_flag)
		return;

	if (wlrx_get_wired_channel_state() != WIRED_CHANNEL_ON) {
		hwlog_info("[stop_charging] irq_abnormal,keep rx_sw on\n");
		irq_abnormal_flag = true;
		wlps_control(di->ic_type, WLPS_RX_SW, true);
	} else {
		di->irq_cnt = 0;
		irq_abnormal_flag = false;
		stwlc68_enable_irq(di);
		hwlog_info("[%s] wired channel on, enable irq_int\n", __func__);
	}
}

static int stwlc68_data_received_handler(struct stwlc68_dev_info *di)
{
	int ret;
	int i;
	u8 cmd;
	u8 buff[QI_PKT_LEN] = { 0 };

	ret = stwlc68_read_block(di, STWLC68_RCVD_MSG_HEADER_ADDR,
		buff, QI_PKT_LEN);
	if (ret) {
		hwlog_err("%s: read data received from TX fail\n", __func__);
		return -WLC_ERR_I2C_R;
	}

	cmd = buff[QI_PKT_CMD];
	hwlog_info("[data_received_handler] cmd: 0x%x\n", cmd);
	for (i = QI_PKT_DATA; i < QI_PKT_LEN; i++)
		hwlog_info("[data_received_handler] data: 0x%x\n", buff[i]);

	switch (cmd) {
	case QI_CMD_TX_ALARM:
	case QI_CMD_ACK_BST_ERR:
		di->irq_val &= ~STWLC68_RX_RCVD_MSG_INTR_LATCH;
		if (g_stwlc68_handle &&
			g_stwlc68_handle->hdl_non_qi_fsk_pkt)
			g_stwlc68_handle->hdl_non_qi_fsk_pkt(buff, QI_PKT_LEN, di);
		break;
	default:
		break;
	}
	return 0;
}

static void stwlc68_rx_ready_handler(struct stwlc68_dev_info *di)
{
	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON) {
		hwlog_err("%s: wired channel on, ignore\n", __func__);
		return;
	}

	hwlog_info("%s rx ready, goto wireless charging\n", __func__);
	stop_charging_flag = 0;
	di->irq_cnt = 0;
	wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	msleep(CHANNEL_SW_TIME);
	stwlc68_fw_sram_update(di, WIRELESS_RX);
	gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
	power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_READY, NULL);
}

static void stwlc68_handle_abnormal_irq(struct stwlc68_dev_info *di)
{
	static struct timespec64 ts64_timeout;
	struct timespec64 ts64_interval;
	struct timespec64 ts64_now;

	ts64_now = current_kernel_time64();
	ts64_interval.tv_sec = 0;
	ts64_interval.tv_nsec = WIRELESS_INT_TIMEOUT_TH * NSEC_PER_MSEC;

	hwlog_info("[%s] irq_cnt = %d\n", __func__, ++di->irq_cnt);
	/* power on interrupt happen first time, so start monitor it */
	if (di->irq_cnt == 1) {
		ts64_timeout = timespec64_add_safe(ts64_now, ts64_interval);
		if (ts64_timeout.tv_sec == TIME_T_MAX) {
			di->irq_cnt = 0;
			hwlog_err("%s: time overflow happened\n", __func__);
			return;
		}
	}

	if (timespec64_compare(&ts64_now, &ts64_timeout) >= 0) {
		if (di->irq_cnt >= WIRELESS_INT_CNT_TH) {
			irq_abnormal_flag = true;
			wlps_control(di->ic_type, WLPS_RX_SW, true);
			stwlc68_disable_irq_nosync(di);
			gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_DISABLE);
			hwlog_err("%s: more than %d irq in %ds, disable irq\n",
			    __func__, WIRELESS_INT_CNT_TH,
			    WIRELESS_INT_TIMEOUT_TH / WL_MSEC_PER_SEC);
		} else {
			di->irq_cnt = 0;
			hwlog_info("%s: less than %d irq in %ds, clr irq cnt\n",
			    __func__, WIRELESS_INT_CNT_TH,
			    WIRELESS_INT_TIMEOUT_TH / WL_MSEC_PER_SEC);
		}
	}
}

static void stwlc68_rx_power_on_handler(struct stwlc68_dev_info *di)
{
	u8 rx_ss = 0; /* ss: Signal Strength */
	int pwr_flag = RX_PWR_ON_NOT_GOOD;

	if (wlrx_get_wired_channel_state() == WIRED_CHANNEL_ON) {
		hwlog_err("rx_power_on_handler: wired channel on, ignore\n");
		return;
	}

	stwlc68_handle_abnormal_irq(di);
	(void)stwlc68_read_byte(STWLC68_RX_SS_ADDR, &rx_ss);
	hwlog_info("[rx_power_on_handler] SS = %u\n", rx_ss);
	if ((rx_ss > di->rx_ss_good_lth) && (rx_ss <= STWLC68_RX_SS_MAX))
		pwr_flag = RX_PWR_ON_GOOD;
	power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_PWR_ON, &pwr_flag);
}

static void stwlc68_rx_mode_irq_recheck(
	struct stwlc68_dev_info *di, u16 irq_val)
{
	if (irq_val & STWLC68_RX_STATUS_READY)
		stwlc68_rx_ready_handler(di);
}

static void stwlc68_rx_mode_irq_handler(struct stwlc68_dev_info *di)
{
	int ret;
	int i;
	u16 irq_value = 0;

	ret = stwlc68_read_word(STWLC68_INTR_LATCH_ADDR, &di->irq_val);
	if (ret) {
		hwlog_err("%s: read interrupt fail, clear\n", __func__);
		stwlc68_clear_interrupt(STWLC68_ALL_INTR_CLR);
		stwlc68_handle_abnormal_irq(di);
		goto clr_irq_again;
	}

	hwlog_info("[%s] interrupt 0x%04x\n", __func__, di->irq_val);
	if (di->irq_val == STWLC68_ABNORMAL_INTR) {
		hwlog_err("%s: abnormal interrupt\n", __func__);
		stwlc68_clear_interrupt(STWLC68_ALL_INTR_CLR);
		stwlc68_handle_abnormal_irq(di);
		goto clr_irq_again;
	}

	stwlc68_clear_interrupt(di->irq_val);

	if (di->irq_val & STWLC68_RX_STATUS_POWER_ON) {
		di->irq_val &= ~STWLC68_RX_STATUS_POWER_ON;
		stwlc68_rx_power_on_handler(di);
	}
	if (di->irq_val & STWLC68_RX_STATUS_READY) {
		di->irq_val &= ~STWLC68_RX_STATUS_READY;
		stwlc68_rx_ready_handler(di);
	}
	if (di->irq_val & STWLC68_OCP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_OCP_INTR_LATCH;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OCP, NULL);
	}
	if (di->irq_val & STWLC68_OVP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_OVP_INTR_LATCH;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OVP, NULL);
	}
	if (di->irq_val & STWLC68_OVTP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_OVTP_INTR_LATCH;
		power_event_bnc_notify(POWER_BNT_WLRX, POWER_NE_WLRX_OTP, NULL);
	}

	/* receice data from TX, please handler the interrupt */
	if (di->irq_val & STWLC68_RX_RCVD_MSG_INTR_LATCH)
		stwlc68_data_received_handler(di);

	if (di->irq_val & STWLC68_SYS_ERR_INTR_LATCH) {
		di->irq_val &= ~STWLC68_SYS_ERR_INTR_LATCH;
		hwlog_info("[%s] SYS_ERR_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_OUTPUT_ON_INTR_LATCH) {
		di->irq_val &= ~STWLC68_OUTPUT_ON_INTR_LATCH;
		hwlog_info("[%s] OUTPUT_ON_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_OUTPUT_OFF_INTR_LATCH) {
		di->irq_val &= ~STWLC68_OUTPUT_OFF_INTR_LATCH;
		hwlog_info("[%s] OUTPUT_OFF_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_SEND_PKT_SUCC_INTR_LATCH) {
		di->irq_val &= ~STWLC68_SEND_PKT_SUCC_INTR_LATCH;
		hwlog_info("[%s] SEND_PKT_SUCC_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_SEND_PKT_TIMEOUT_INTR_LATCH) {
		di->irq_val &= ~STWLC68_SEND_PKT_TIMEOUT_INTR_LATCH;
		hwlog_info("[%s] SEND_PKT_TIMEOUT_INTR_LATCH\n", __func__);
	}

clr_irq_again:
	/* clear interrupt again */
	for (i = 0; (!gpio_get_value(di->gpio_int)) && (i < STWLC68_INTR_CLR_CNT); i++) {
		stwlc68_read_word(STWLC68_INTR_LATCH_ADDR, &irq_value);
		stwlc68_rx_mode_irq_recheck(di, irq_value);
		hwlog_info("[rx_irq_handler] gpio_int low, clear irq cnt=%d irq=0x%x\n",
			i, irq_value);
		stwlc68_clear_interrupt(STWLC68_ALL_INTR_CLR);
		power_usleep(DT_USLEEP_1MS); /* delay for gpio int pull up */
	}
}

static bool stwlc68_in_tx_mode(void *dev_data)
{
	int ret;
	u8 mode = 0;

	ret = stwlc68_read_byte(STWLC68_OP_MODE_ADDR, &mode);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return false;
	}

	return mode == STWLC68_TX_MODE;
}

static int stwlc68_set_tx_open_flag(bool enable, void *dev_data)
{
	g_tx_open_flag = enable;
	return 0;
}

static int stwlc68_set_tx_vset(int tx_vset, void *dev_data)
{
	if (tx_vset == STWLC68_PS_TX_VOLT_5V5)
		return stwlc68_write_byte(STWLC68_PS_TX_GPIO_ADDR,
			STWLC68_PS_TX_GPIO_PU);
	else if ((tx_vset == STWLC68_PS_TX_VOLT_6V8) ||
		(tx_vset == STWLC68_PS_TX_VOLT_6V))
		return stwlc68_write_byte(STWLC68_PS_TX_GPIO_ADDR,
			STWLC68_PS_TX_GPIO_OPEN);
	else if ((tx_vset == STWLC68_PS_TX_VOLT_10V) ||
		(tx_vset == STWLC68_PS_TX_VOLT_6V9))
		return stwlc68_write_byte(STWLC68_PS_TX_GPIO_ADDR,
			STWLC68_PS_TX_GPIO_PD);

	hwlog_err("%s: para err\n", __func__);
	return -WLC_ERR_PARA_WRONG;
}

static bool stwlc68_check_rx_disconnect(void *dev_data)
{
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return true;

	if (di->ept_type & STWLC68_TX_EPT_SRC_CEP_TIMEOUT) {
		di->ept_type &= ~STWLC68_TX_EPT_SRC_CEP_TIMEOUT;
		hwlog_info("[%s] RX disconnect\n", __func__);
		return true;
	}

	return false;
}

static int stwlc68_get_tx_ping_interval(u16 *ping_interval, void *dev_data)
{
	int ret;
	u8 data = 0;

	if (!ping_interval) {
		hwlog_err("%s: ping_interval null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_byte(STWLC68_TX_PING_INTERVAL, &data);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}
	*ping_interval = data * STWLC68_TX_PING_INTERVAL_STEP;

	return 0;
}

static int stwlc68_set_tx_ping_interval(u16 ping_interval, void *dev_data)
{
	if ((ping_interval < STWLC68_TX_PING_INTERVAL_MIN) ||
		(ping_interval > STWLC68_TX_PING_INTERVAL_MAX)) {
		hwlog_err("%s: ping interval is out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	return stwlc68_write_byte(STWLC68_TX_PING_INTERVAL,
		(u8)(ping_interval / STWLC68_TX_PING_INTERVAL_STEP));
}

static int stwlc68_get_tx_ping_freq(u16 *ping_freq, void *dev_data)
{
	return stwlc68_read_byte(STWLC68_TX_PING_FREQ_ADDR, (u8 *)ping_freq);
}

static int stwlc68_set_tx_ping_freq(u16 ping_freq, void *dev_data)
{
	if ((ping_freq < STWLC68_TX_PING_FREQ_MIN) ||
		(ping_freq > STWLC68_TX_PING_FREQ_MAX)) {
		hwlog_err("%s: ping frequency is out of range\n", __func__);
		return -WLC_ERR_PARA_WRONG;
	}

	return stwlc68_write_byte(STWLC68_TX_PING_FREQ_ADDR, (u8)ping_freq);
}

static int stwlc68_get_tx_min_fop(u16 *fop, void *dev_data)
{
	return stwlc68_read_byte(STWCL68_TX_MIN_FOP_ADDR, (u8 *)fop);
}

static int stwlc68_set_tx_min_fop(u16 fop, void *dev_data)
{
	if ((fop < STWLC68_TX_MIN_FOP_VAL) || (fop > STWLC68_TX_MAX_FOP_VAL))
		return -WLC_ERR_PARA_WRONG;

	return stwlc68_write_byte(STWCL68_TX_MIN_FOP_ADDR, (u8)fop);
}

static int stwlc68_get_tx_max_fop(u16 *fop, void *dev_data)
{
	return stwlc68_read_byte(STWLC68_TX_MAX_FOP_ADDR, (u8 *)fop);
}

static int stwlc68_set_tx_max_fop(u16 fop, void *dev_data)
{
	if ((fop < STWLC68_TX_MIN_FOP_VAL) || (fop > STWLC68_TX_MAX_FOP_VAL))
		return -WLC_ERR_PARA_WRONG;

	return stwlc68_write_byte(STWLC68_TX_MAX_FOP_ADDR, (u8)fop);
}

static int stwlc68_get_tx_fop(u16 *fop, void *dev_data)
{
	return stwlc68_read_word(STWLC68_TX_OP_FREQ_ADDR, fop);
}

static int stwlc68_tx_get_duty(u8 *duty, void *dev_data)
{
	*duty = 50; /* default 50, full duty */
	return 0;
}

static int stwlc68_tx_get_cep(s8 *cep, void *dev_data)
{
	return stwlc68_read_byte(STWLC68_TX_CEP_ADDR, cep);
}

static int stwlc68_tx_get_ptx(u32 *ptx, void *dev_data)
{
	return stwlc68_read_word(STWLC68_TX_TFRD_PWR_ADDR, (u16 *)ptx);
}

static int stwlc68_tx_get_prx(u32 *prx, void *dev_data)
{
	return stwlc68_read_word(STWLC68_TX_RCVD_PWR_ADDR, (u16 *)prx);
}

static int stwlc68_tx_get_ploss(s32 *ploss, void *dev_data)
{
	int ret;
	u32 ptx = 0;
	u32 prx = 0;

	if (!ploss)
		return -EINVAL;

	ret = stwlc68_tx_get_ptx(&ptx, dev_data);
	ret += stwlc68_tx_get_prx(&prx, dev_data);
	if (ret)
		return -EIO;

	*ploss = ptx - prx;
	return 0;
}

static int stwlc68_tx_get_ploss_id(u8 *id, void *dev_data)
{
	*id = 0; /* default 0, 5v ploss */
	return 0;
}

static int stwlc68_get_tx_temp(s16 *chip_temp, void *dev_data)
{
	return stwlc68_read_word(STWLC68_CHIP_TEMP_ADDR, chip_temp);
}

static int stwlc68_get_tx_vin(u16 *tx_vin, void *dev_data)
{
	return stwlc68_read_word(STWLC68_TX_VIN_ADDR, tx_vin);
}

static int stwlc68_get_tx_vrect(u16 *tx_vrect, void *dev_data)
{
	return stwlc68_read_word(STWLC68_TX_VRECT_ADDR, tx_vrect);
}

static int stwlc68_get_tx_iin(u16 *tx_iin, void *dev_data)
{
	return stwlc68_read_word(STWLC68_TX_IIN_ADDR, tx_iin);
}

static int stwlc68_set_tx_ilimit(u16 tx_ilim, void *dev_data)
{
	int ret;

	if ((tx_ilim < STWLC68_TX_ILIMIT_MIN) ||
		(tx_ilim > STWLC68_TX_ILIMIT_MAX))
		return -WLC_ERR_PARA_WRONG;

	ret = stwlc68_write_byte(STWLC68_TX_ILIMIT,
		(u8)(tx_ilim / STWLC68_TX_ILIMIT_STEP));
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_set_tx_fod_coef(u16 pl_th, u8 pl_cnt, void *dev_data)
{
	int ret;

	pl_th /= STWLC68_TX_PLOSS_TH_UNIT;
	/* tx ploss threshold 0:disabled */
	ret = stwlc68_write_byte(STWLC68_TX_PLOSS_TH_ADDR, (u8)pl_th);
	/* tx ploss fod debounce count 0:no debounce */
	ret += stwlc68_write_byte(STWLC68_TX_PLOSS_CNT_ADDR, pl_cnt);

	return ret;
}

static int stwlc68_set_rp_dm_timeout_val(u8 val, void *dev_data)
{
	if (stwlc68_write_byte(STWLC68_TX_RP_TIMEOUT_ADDR, val))
		hwlog_err("%s fail\n", __func__);

	return 0;
}

static int stwlc68_tx_stop_config(void *dev_data)
{
	return 0;
}

static int stwlc68_tx_chip_init(unsigned int client, void *dev_data)
{
	int ret;
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	di->irq_cnt = 0;
	irq_abnormal_flag = false;
	/*
	 * when stwlc68_handle_abnormal_irq disable irq, tx will irq-mask
	 * so, here enable irq
	 */
	stwlc68_enable_irq(di);

	ret = stwlc68_write_byte(STWLC68_TX_OTP_ADDR, STWLC68_TX_OTP_THRES);
	ret += stwlc68_write_byte(STWLC68_TX_OCP_ADDR,
		di->tx_ocp_val / STWLC68_TX_OCP_STEP);
	ret += stwlc68_write_byte(STWLC68_TX_OVP_ADDR,
		di->tx_ovp_val / STWLC68_TX_OVP_STEP);
	ret += stwlc68_write_word_mask(STWLC68_CMD_ADDR, STWLC68_CMD_TX_FOD_EN,
		STWLC68_CMD_TX_FOD_EN_SHIFT, STWLC68_CMD_VAL);
	ret += stwlc68_set_tx_ping_freq(STWLC68_TX_PING_FREQ_INIT, di);
	ret += stwlc68_set_tx_min_fop(STWLC68_TX_MIN_FOP_VAL, di);
	ret += stwlc68_set_tx_max_fop(STWLC68_TX_MAX_FOP_VAL, di);
	ret += stwlc68_set_tx_ping_interval(STWLC68_TX_PING_INTERVAL_INIT, di);
	ret += stwlc68_write_byte(STWLC68_TX_UVP_ADDR, di->tx_uvp_th / STWLC68_TX_UVP_STEP);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static int stwlc68_enable_tx_mode(bool enable, void *dev_data)
{
	int ret;
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (enable)
		ret = stwlc68_write_word_mask(STWLC68_CMD_ADDR,
			STWLC68_CMD_TX_EN, STWLC68_CMD_TX_EN_SHIFT,
			STWLC68_CMD_VAL);
	else
		ret = stwlc68_write_word_mask(STWLC68_CMD_ADDR,
			STWLC68_CMD_TX_DIS, STWLC68_CMD_TX_DIS_SHIFT,
			STWLC68_CMD_VAL);

	if (ret) {
		hwlog_err("%s: %s tx mode fail\n", __func__,
			  enable ? "enable" : "disable");
		return ret;
	}

	return 0;
}

static int stwlc68_send_fsk_msg(u8 cmd, u8 *data, int data_len, void *dev_data)
{
	int ret;
	u8 header;
	u8 write_data[STWLC68_SEND_MSG_DATA_LEN] = { 0 };
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if ((data_len > STWLC68_SEND_MSG_DATA_LEN) || (data_len < 0)) {
		hwlog_err("%s: data len out of range\n", __func__);
		return -EINVAL;
	}

	if (cmd == STWLC68_CMD_ACK)
		header = STWLC68_CMD_ACK_HEAD;
	else
		header = stwlc68_tx2rx_header[data_len + 1];

	ret = stwlc68_write_byte(STWLC68_SEND_MSG_HEADER_ADDR, header);
	if (ret) {
		hwlog_err("%s: write header fail\n", __func__);
		return ret;
	}
	ret = stwlc68_write_byte(STWLC68_SEND_MSG_CMD_ADDR, cmd);
	if (ret) {
		hwlog_err("%s: write cmd fail\n", __func__);
		return ret;
	}

	if (data && data_len > 0) {
		memcpy(write_data, data, data_len);
		ret = stwlc68_write_block(di, STWLC68_SEND_MSG_DATA_ADDR,
			write_data, data_len);
		if (ret) {
			hwlog_err("%s: write fsk reg fail\n", __func__);
			return ret;
		}
	}
	ret = stwlc68_write_word_mask(STWLC68_CMD_ADDR, STWLC68_CMD_SEND_MSG,
		STWLC68_CMD_SEND_MSG_SHIFT, STWLC68_CMD_VAL);
	if (ret) {
		hwlog_err("%s: send fsk fail\n", __func__);
		return ret;
	}

	hwlog_info("[%s] success\n", __func__);
	return 0;
}

static int stwlc68_get_tx_ept_type(u16 *ept_type)
{
	int ret;
	u16 data = 0;

	if (!ept_type) {
		hwlog_err("%s: ept_type null\n", __func__);
		return -WLC_ERR_PARA_NULL;
	}

	ret = stwlc68_read_word(STWLC68_TX_EPT_REASON_RCVD_ADDR, &data);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return ret;
	}
	*ept_type = data;
	hwlog_info("[%s] EPT type = 0x%04x", __func__, *ept_type);
	ret = stwlc68_write_word(STWLC68_TX_EPT_REASON_RCVD_ADDR, 0);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return ret;
	}

	return 0;
}

static int stwlc68_get_ask_pkt(u8 *pkt_data, int pkt_data_len, void *dev_data)
{
	int ret;
	int i;
	char buff[STWLC68_RCVD_PKT_BUFF_LEN] = { 0 };
	char pkt_str[STWLC68_RCVD_PKT_STR_LEN] = { 0 };
	struct stwlc68_dev_info *di = g_stwlc68_di;

	if (!di || !pkt_data || (pkt_data_len <= 0) ||
		(pkt_data_len > STWLC68_RCVD_MSG_PKT_LEN)) {
		hwlog_err("%s: para err\n", __func__);
		return -1;
	}
	ret = stwlc68_read_block(di, STWLC68_RCVD_MSG_HEADER_ADDR,
		pkt_data, pkt_data_len);
	if (ret) {
		hwlog_err("%s: read fail\n", __func__);
		return -1;
	}
	for (i = 0; i < pkt_data_len; i++) {
		snprintf(buff, STWLC68_RCVD_PKT_BUFF_LEN, "0x%02x ",
			pkt_data[i]);
		strncat(pkt_str, buff, strlen(buff));
	}
	hwlog_info("[%s] RX back packet: %s\n", __func__, pkt_str);
	return 0;
}

static void stwlc68_handle_tx_ept(struct stwlc68_dev_info *di)
{
	int ret;

	ret = stwlc68_get_tx_ept_type(&di->ept_type);
	if (ret) {
		hwlog_err("%s: get tx ept type fail\n", __func__);
		return;
	}
	switch (di->ept_type) {
	case STWLC68_TX_EPT_SRC_SS:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_SS;
		hwlog_info("[%s] SS timeout\n", __func__);
		break;
	case STWLC68_TX_EPT_SRC_ID:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_ID;
		hwlog_info("[%s] ID packet error\n", __func__);
		break;
	case STWLC68_TX_EPT_SRC_XID:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_XID;
		hwlog_info("[%s] XID packet error\n", __func__);
		break;
	case STWLC68_TX_EPT_SRC_CFG_CNT_ERR:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_CFG_CNT_ERR;
		hwlog_info("[%s] wrong CFG optional pkt number\n", __func__);
		break;
	case STWLC68_TX_EPT_SRC_CFG_ERR:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_CFG_ERR;
		hwlog_info("[%s] CFG pkt error\n", __func__);
		break;
	case STWLC68_TX_EPT_SRC_RPP_TIMEOUT:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_RPP_TIMEOUT;
		hwlog_info("[%s] RPP pkt timeout\n", __func__);
		break;
	case STWLC68_TX_EPT_SRC_CEP_TIMEOUT:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_CEP_TIMEOUT;
		hwlog_info("[%s] RX disconnect\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_CEP_TIMEOUT, NULL);
		break;
	case STWLC68_TX_EPT_SRC_FOD:
		di->ept_type &= ~STWLC68_TX_EPT_SRC_FOD;
		hwlog_info("[%s] fod happened\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_TX_FOD, NULL);
		break;
	case STWLC68_TX_EPT_SRC_RX_EPT:
		hwlog_info("[handle_tx_ept] rx ept\n");
		if (di->dev_type == WIRELESS_DEVICE_PAD)
			power_event_bnc_notify(POWER_BNT_WLTX,
				POWER_NE_WLTX_CEP_TIMEOUT, NULL);
		break;
	default:
		break;
	}
}

static int stwlc68_clear_tx_interrupt(u16 itr)
{
	int ret;

	ret = stwlc68_write_word(STWLC68_TX_INTR_CLR_ADDR, itr);
	if (ret) {
		hwlog_err("%s: write fail\n", __func__);
		return -WLC_ERR_I2C_W;
	}

	return 0;
}

static void stwlc68_tx_mode_irq_handler(struct stwlc68_dev_info *di)
{
	int ret;
	int i;
	u16 irq_value = 0;

	ret = stwlc68_read_word(STWLC68_TX_INTR_LATCH_ADDR, &di->irq_val);
	if (ret) {
		hwlog_err("%s: read interrupt fail, clear\n", __func__);
		stwlc68_clear_tx_interrupt(STWLC68_ALL_INTR_CLR);
		goto clr_irq_again;
	}

	hwlog_info("[%s] interrupt 0x%04x\n", __func__, di->irq_val);
	if (di->irq_val == STWLC68_ABNORMAL_INTR) {
		hwlog_err("%s: abnormal interrupt\n", __func__);
		stwlc68_clear_tx_interrupt(STWLC68_ALL_INTR_CLR);
		goto clr_irq_again;
	}

	stwlc68_clear_tx_interrupt(di->irq_val);

	if (di->irq_val & STWLC68_TX_START_PING_LATCH) {
		hwlog_info("%s: tx PING interrupt\n", __func__);
		di->irq_val &= ~STWLC68_TX_START_PING_LATCH;
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_PING_RX, NULL);
	}

	if (di->irq_val & STWLC68_TX_SS_PKG_RCVD_LATCH) {
		hwlog_info("%s: Signal Strength packet interrupt\n", __func__);
		di->irq_val &= ~STWLC68_TX_SS_PKG_RCVD_LATCH;
		if (g_stwlc68_handle && g_stwlc68_handle->hdl_qi_ask_pkt)
			g_stwlc68_handle->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & STWLC68_TX_ID_PCKET_RCVD_LATCH) {
		hwlog_info("%s: ID packet interrupt\n", __func__);
		di->irq_val &= ~STWLC68_TX_ID_PCKET_RCVD_LATCH;
		if (g_stwlc68_handle && g_stwlc68_handle->hdl_qi_ask_pkt)
			g_stwlc68_handle->hdl_qi_ask_pkt(di);
	}

	if (di->irq_val & STWLC68_TX_CFG_PKT_RCVD_LATCH) {
		hwlog_info("%s: Config packet interrupt\n", __func__);
		di->irq_val &= ~STWLC68_TX_CFG_PKT_RCVD_LATCH;
		if (g_stwlc68_handle && g_stwlc68_handle->hdl_qi_ask_pkt)
			g_stwlc68_handle->hdl_qi_ask_pkt(di);
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_GET_CFG, NULL);
	}
	if (di->irq_val & STWLC68_TX_EPT_PKT_RCVD_LATCH) {
		di->irq_val &= ~STWLC68_TX_EPT_PKT_RCVD_LATCH;
		stwlc68_handle_tx_ept(di);
	}

	if (di->irq_val & STWLC68_TX_PP_PKT_RCVD_LATCH) {
		hwlog_info("[%s] TX rcv personal property ASK pkt\n", __func__);
		di->irq_val &= ~STWLC68_TX_PP_PKT_RCVD_LATCH;
		if (g_stwlc68_handle && g_stwlc68_handle->hdl_non_qi_ask_pkt)
			g_stwlc68_handle->hdl_non_qi_ask_pkt(di);
	}

	if (di->irq_val & STWLC68_TX_OVTP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_TX_OVTP_INTR_LATCH;
		hwlog_info("[%s] TX_OVTP_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_TX_OCP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_TX_OCP_INTR_LATCH;
		hwlog_info("[%s] TX_OCP_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_TX_OVP_INTR_LATCH) {
		di->irq_val &= ~STWLC68_TX_OVP_INTR_LATCH;
		hwlog_info("[%s] TX_OVP_INTR_LATCH\n", __func__);
		if (di->dev_type == WIRELESS_DEVICE_PAD)
			power_event_bnc_notify(POWER_BNT_WLTX,
				POWER_NE_WLTX_OVP, NULL);
	}

	if (di->irq_val & STWLC68_TX_SYS_ERR_INTR_LATCH) {
		di->irq_val &= ~STWLC68_TX_SYS_ERR_INTR_LATCH;
		hwlog_info("[%s] TX_SYS_ERR_INTR_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_TX_EXT_MON_INTR_LATCH) {
		di->irq_val &= ~STWLC68_TX_EXT_MON_INTR_LATCH;
		hwlog_info("rcv dping in tx mode\n");
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_RCV_DPING, NULL);
	}

	if (di->irq_val & STWLC68_TX_SEND_PKT_SUCC_LATCH) {
		di->irq_val &= ~STWLC68_TX_SEND_PKT_SUCC_LATCH;
		hwlog_info("[%s] TX_SEND_PKT_SUCC_LATCH\n", __func__);
	}

	if (di->irq_val & STWLC68_TX_SEND_PKT_TIMEOUT_LATCH) {
		di->irq_val &= ~STWLC68_TX_SEND_PKT_TIMEOUT_LATCH;
		hwlog_info("[%s] TX_SEND_PKT_TIMEOUT_LATCH\n", __func__);
	}
	if (di->irq_val & STWLC68_TX_RP_DM_TIMEOUT_LATCH) {
		di->irq_val &= ~STWLC68_TX_RP_DM_TIMEOUT_LATCH;
		hwlog_info("[%s] TX_RP_DM_TIMEOUT_LATCH\n", __func__);
		power_event_bnc_notify(POWER_BNT_WLTX,
			POWER_NE_WLTX_RP_DM_TIMEOUT, NULL);
	}

clr_irq_again:
	for (i = 0; (!gpio_get_value(di->gpio_int)) && (i < STWLC68_TX_INTR_CLR_CNT); i++) {
		stwlc68_read_word(STWLC68_TX_INTR_LATCH_ADDR, &irq_value);
		hwlog_info("[tx_irq_handler] gpio_int low, clear irq cnt=%d irq=0x%x\n",
			i, irq_value);
		stwlc68_clear_tx_interrupt(STWLC68_ALL_INTR_CLR);
		power_usleep(DT_USLEEP_1MS); /* delay for gpio int pull up */
	}
}


static void stwlc68_irq_work(struct work_struct *work)
{
	int ret;
	int gpio_val;
	u8 mode = 0;
	struct stwlc68_dev_info *di =
		container_of(work, struct stwlc68_dev_info, irq_work);

	if (!di)
		goto exit;

	gpio_val = gpio_get_value(di->gpio_en);
	if (gpio_val != di->gpio_en_valid_val) {
		hwlog_err("[%s] gpio is %s now\n",
		__func__, gpio_val ? "high" : "low");
		goto exit;
	}
	gpio_val = gpio_get_value(di->gpio_int);
	if (gpio_val != STWLC68_IRQ_INT_VALID_VAL) {
		hwlog_err("[%s] gpio_int is %s now\n",
			__func__, gpio_val ? "high" : "low");
		goto exit;
	}
	/* get System Operating Mode */
	ret = stwlc68_get_mode(&mode);
	if (!ret)
		hwlog_info("[%s] mode = 0x%x\n", __func__, mode);
	else
		stwlc68_handle_abnormal_irq(di);

	/* handler interrupt */
	if ((mode == STWLC68_TX_MODE) || (mode == STWLC68_STANDALONE_MODE))
		stwlc68_tx_mode_irq_handler(di);
	else if (mode == STWLC68_RX_MODE)
		stwlc68_rx_mode_irq_handler(di);

exit:
	if (!irq_abnormal_flag && di)
		stwlc68_enable_irq(di);

	stwlc68_wake_unlock();
}

static irqreturn_t stwlc68_interrupt(int irq, void *_di)
{
	struct stwlc68_dev_info *di = _di;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return IRQ_HANDLED;
	}

	stwlc68_wake_lock();
	hwlog_info("[%s] ++\n", __func__);
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = 0;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("irq is not enable,do nothing\n");
		stwlc68_wake_unlock();
	}
	hwlog_info("[%s] --\n", __func__);

	return IRQ_HANDLED;
}

static void stwlc68_pmic_vbus_handler(bool vbus_state, void *dev_data)
{
	int ret;
	int wired_ch_state;
	u16 irq_val = 0;
	struct stwlc68_dev_info *di = dev_data;

	if (!di) {
		hwlog_err("%s: di null\n", __func__);
		return;
	}
	wired_ch_state = wlrx_get_wired_channel_state();
	if (vbus_state && irq_abnormal_flag &&
		(wired_ch_state != WIRED_CHANNEL_ON) &&
		stwlc68_is_tx_exist(dev_data)) {
		ret = stwlc68_read_word(STWLC68_INTR_LATCH_ADDR, &irq_val);
		if (ret) {
			hwlog_err("%s: read interrupt fail, clear\n", __func__);
			return;
		}
		hwlog_info("[%s] irq_val = 0x%x\n", __func__, irq_val);
		if (irq_val & STWLC68_RX_STATUS_READY) {
			stwlc68_clear_interrupt(WORD_MASK);
			stwlc68_rx_ready_handler(di);
			di->irq_cnt = 0;
			irq_abnormal_flag = false;
			stwlc68_enable_irq(di);
		}
	}
}

static int stwlc68_dev_check(struct stwlc68_dev_info *di)
{
	int ret;
	u16 chip_id = 0;

	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	usleep_range(9500, 10500); /* 10ms */
	di->g_val.sram_chk_complete = true;
	ret = stwlc68_get_chip_id(&chip_id);
	if (ret) {
		hwlog_err("%s: fail\n", __func__);
		wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
		di->g_val.sram_chk_complete = false;
		return ret;
	}
	di->g_val.sram_chk_complete = false;
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);

	hwlog_info("[%s] chip_id = %d\n", __func__, chip_id);

	if (chip_id != STWLC68_CHIP_ID)
		hwlog_err("%s: wlc_chip not match\n", __func__);

	return 0;
}

static int stwlc68_parse_fod(struct device_node *np,
	struct stwlc68_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_5v", di->rx_fod_5v, STWLC68_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"pu_rx_fod_5v", di->pu_rx_fod_5v, STWLC68_RX_FOD_LEN))
		di->need_chk_pu_shell = false;
	else
		di->need_chk_pu_shell = true;
	hwlog_info("[%s] need_chk_pu_shell=%d\n",
		__func__, di->need_chk_pu_shell);

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_9v", di->rx_fod_9v, STWLC68_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_12v", di->rx_fod_12v, STWLC68_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_15v", di->rx_fod_15v, STWLC68_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_9v_cp60", di->rx_fod_9v_cp60, STWLC68_RX_FOD_LEN))
		memcpy(di->rx_fod_9v_cp60, di->rx_fod_9v,
			sizeof(di->rx_fod_9v));

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_9v_cp39s", di->rx_fod_9v_cp39s, STWLC68_RX_FOD_LEN))
		memcpy(di->rx_fod_9v_cp39s, di->rx_fod_9v,
			sizeof(di->rx_fod_9v));

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_15v_cp39s", di->rx_fod_15v_cp39s, STWLC68_RX_FOD_LEN))
		memcpy(di->rx_fod_15v_cp39s, di->rx_fod_15v,
			sizeof(di->rx_fod_15v));

	return 0;
}

static int stwlc68_parse_ldo_cfg(struct device_node *np,
	struct stwlc68_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_5v", di->rx_ldo_cfg_5v, STWLC68_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_9v", di->rx_ldo_cfg_9v, STWLC68_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_12v", di->rx_ldo_cfg_12v, STWLC68_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_sc", di->rx_ldo_cfg_sc, STWLC68_LDO_CFG_LEN))
		return -EINVAL;

	return 0;
}

static struct device_node *stwlc68_dts_dev_node(void *dev_data)
{
	struct stwlc68_dev_info *di = dev_data;

	if (!di || !di->dev)
		return NULL;

	return di->dev->of_node;
}

static void stwlc68_parse_rx_offset(struct device_node *np,
	struct stwlc68_dev_info *di)
{
	(void)power_dts_read_u8(power_dts_tag(HWLOG_TAG), np,
		"rx_offset_9v", &di->rx_offset_9v,
		STWLC68_RX_OFFSET_DEFAULT_VALUE);

	(void)power_dts_read_u8(power_dts_tag(HWLOG_TAG), np,
		"rx_offset_15v", &di->rx_offset_15v,
		STWLC68_RX_OFFSET_DEFAULT_VALUE);
}

static int stwlc68_parse_dts(struct device_node *np,
	struct stwlc68_dev_info *di)
{
	int ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ss_good_lth", (u32 *)&di->rx_ss_good_lth,
		STWLC68_RX_SS_MAX);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", (u32 *)&di->gpio_en_valid_val,
		WLTRX_IC_EN_ENABLE);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_cp", (u32 *)&di->support_cp, 1); /* 1: support cp */

	ret = stwlc68_parse_fod(np, di);
	if (ret) {
		hwlog_err("%s: parse fod para fail\n", __func__);
		return ret;
	}
	stwlc68_parse_rx_offset(np, di);
	ret = stwlc68_parse_ldo_cfg(np, di);
	if (ret) {
		hwlog_err("%s: parse ldo cfg fail\n", __func__);
		return ret;
	}
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"dev_type", (u32 *)&di->dev_type, WIRELESS_DEVICE_UNKNOWN);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ocp_val", (u32 *)&di->tx_ocp_val, STWLC68_TX_OCP_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ovp_val", (u32 *)&di->tx_ovp_val, STWLC68_TX_OVP_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_uvp_th", (u32 *)&di->tx_uvp_th, 0);
	return 0;
}

static int stwlc68_gpio_init(struct stwlc68_dev_info *di,
	struct device_node *np)
{
	int ret;

	/* gpio_en */
	if (power_gpio_config_output(np, "gpio_en", "stwlc68_en",
		&di->gpio_en, di->gpio_en_valid_val)) {
		ret = -EINVAL;
		goto gpio_en_fail;
	}

	/* gpio_sleep_en */
	if (power_gpio_config_output(np, "gpio_sleep_en", "stwlc68_sleep_en",
		&di->gpio_sleep_en, RX_SLEEP_EN_DISABLE)) {
		ret = -EINVAL;
		goto gpio_sleep_en_fail;
	}

	/* gpio_pwr_good */
	di->gpio_pwr_good = of_get_named_gpio(np, "gpio_pwr_good", 0);
	if (di->gpio_pwr_good <= 0)
		return 0;
	if (!gpio_is_valid(di->gpio_pwr_good)) {
		hwlog_err("%s: gpio_pwr_good not valid\n", __func__);
		ret = -EINVAL;
		goto gpio_pwr_good_fail;
	}
	hwlog_info("[%s] gpio_pwr_good = %d\n", __func__, di->gpio_pwr_good);
	ret = gpio_request(di->gpio_pwr_good, "stwlc68_pwr_good");
	if (ret) {
		hwlog_err("%s: could not request stwlc68_pwr_good\n", __func__);
		goto gpio_pwr_good_fail;
	}
	ret = gpio_direction_input(di->gpio_pwr_good);
	if (ret) {
		hwlog_err("%s: set gpio_pwr_good input fail\n", __func__);
		goto pwr_good_dir_fail;
	}

	return 0;

pwr_good_dir_fail:
	gpio_free(di->gpio_pwr_good);
gpio_pwr_good_fail:
	gpio_free(di->gpio_sleep_en);
gpio_sleep_en_fail:
	gpio_free(di->gpio_en);
gpio_en_fail:
	return ret;
}

static int stwlc68_irq_init(struct stwlc68_dev_info *di,
	struct device_node *np)
{
	int ret;

	if (power_gpio_config_interrupt(np,
		"gpio_int", "stwlc68_int", &di->gpio_int, &di->irq_int)) {
		ret = -EINVAL;
		goto irq_init_fail_0;
	}

	ret = request_irq(di->irq_int, stwlc68_interrupt,
			  IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND,
			  "stwlc68_irq", di);
	if (ret) {
		hwlog_err("could not request stwlc68_irq\n");
		di->irq_int = -EINVAL;
		goto irq_init_fail_1;
	}
	enable_irq_wake(di->irq_int);
	di->irq_active = 1;
	INIT_WORK(&di->irq_work, stwlc68_irq_work);

	return 0;

irq_init_fail_1:
	gpio_free(di->gpio_int);
irq_init_fail_0:
	return ret;
}

static struct wlrx_ic_ops g_stwlc68_rx_ic_ops = {
	.get_dev_node           = stwlc68_dts_dev_node,
	.fw_update              = stwlc68_rx_fw_update,
	.chip_init              = stwlc68_chip_init,
	.chip_reset             = stwlc68_chip_reset,
	.chip_enable            = stwlc68_chip_enable,
	.is_chip_enable         = stwlc68_is_chip_enable,
	.sleep_enable           = stwlc68_sleep_enable,
	.is_sleep_enable        = stwlc68_is_sleep_enable,
	.ext_pwr_prev_ctrl      = stwlc68_ext_pwr_prev_ctrl,
	.get_vrect              = stwlc68_get_rx_vrect,
	.get_vout               = stwlc68_get_rx_vout,
	.get_iout               = stwlc68_get_rx_iout,
	.get_vout_reg           = stwlc68_get_rx_vout_reg,
	.get_vfc_reg            = stwlc68_get_vfc_reg,
	.set_vfc                = stwlc68_set_vfc,
	.set_vout               = stwlc68_set_rx_vout,
	.get_fop                = stwlc68_get_rx_fop,
	.get_cep                = stwlc68_get_rx_cep,
	.get_temp               = stwlc68_get_rx_temp,
	.get_chip_info          = stwlc68_get_chip_info_str,
	.get_fod_coef           = stwlc68_get_rx_fod,
	.set_fod_coef           = stwlc68_set_rx_fod,
	.is_tx_exist            = stwlc68_is_tx_exist,
	.kick_watchdog          = stwlc68_kick_watchdog,
	.send_ept               = stwlc68_send_ept,
	.stop_charging          = stwlc68_stop_charging,
	.pmic_vbus_handler      = stwlc68_pmic_vbus_handler,
	.need_chk_pu_shell      = stwlc68_need_check_pu_shell,
	.set_pu_shell_flag      = stwlc68_set_pu_shell_flag,
};

static struct wltx_ic_ops g_stwlc68_tx_ic_ops = {
	.get_dev_node           = stwlc68_dts_dev_node,
	.fw_update              = stwlc68_tx_fw_update,
	.chip_init              = stwlc68_tx_chip_init,
	.chip_reset             = stwlc68_chip_reset,
	.chip_enable            = stwlc68_chip_enable,
	.mode_enable            = stwlc68_enable_tx_mode,
	.set_open_flag          = stwlc68_set_tx_open_flag,
	.set_stop_cfg           = stwlc68_tx_stop_config,
	.is_rx_discon           = stwlc68_check_rx_disconnect,
	.is_in_tx_mode          = stwlc68_in_tx_mode,
	.is_in_rx_mode          = stwlc68_in_rx_mode,
	.get_vrect              = stwlc68_get_tx_vrect,
	.get_vin                = stwlc68_get_tx_vin,
	.get_iin                = stwlc68_get_tx_iin,
	.get_temp               = stwlc68_get_tx_temp,
	.get_fop                = stwlc68_get_tx_fop,
	.get_cep                = stwlc68_tx_get_cep,
	.get_duty               = stwlc68_tx_get_duty,
	.get_ptx                = stwlc68_tx_get_ptx,
	.get_prx                = stwlc68_tx_get_prx,
	.get_ploss              = stwlc68_tx_get_ploss,
	.get_ploss_id           = stwlc68_tx_get_ploss_id,
	.get_ping_freq          = stwlc68_get_tx_ping_freq,
	.get_ping_interval      = stwlc68_get_tx_ping_interval,
	.get_min_fop            = stwlc68_get_tx_min_fop,
	.get_max_fop            = stwlc68_get_tx_max_fop,
	.set_ping_freq          = stwlc68_set_tx_ping_freq,
	.set_ping_interval      = stwlc68_set_tx_ping_interval,
	.set_min_fop            = stwlc68_set_tx_min_fop,
	.set_max_fop            = stwlc68_set_tx_max_fop,
	.set_ilim               = stwlc68_set_tx_ilimit,
	.set_vset               = stwlc68_set_tx_vset,
	.set_fod_coef           = stwlc68_set_tx_fod_coef,
	.set_rp_dm_to           = stwlc68_set_rp_dm_timeout_val,
};

static int stwlc68_get_chip_fw_version(u8 *data, int len, void *dev_data)
{
	struct stwlc68_chip_info chip_info;

	if (stwlc68_get_chip_info(&chip_info)) {
		hwlog_err("%s: get chip info fail\n", __func__);
		return -1;
	}

	/* fw version length must be 4 */
	if (len != 4)
		return -1;

	/* byte[0:1]=patch_id, byte[2:3]=sram_id */
	data[0] = (u8)((chip_info.patch_id >> 0) & BYTE_MASK);
	data[1] = (u8)((chip_info.patch_id >> BITS_PER_BYTE) & BYTE_MASK);
	data[2] = (u8)((chip_info.sram_id >> 0) & BYTE_MASK);
	data[3] = (u8)((chip_info.sram_id >> BITS_PER_BYTE) & BYTE_MASK);
	return 0;
}

static int stwlc68_get_tx_id_pre(void *dev_data)
{
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (!power_cmdline_is_factory_mode() &&
		!stwlc68_fw_sram_update(di, WIRELESS_RX))
		(void)stwlc68_chip_init(WIRELESS_CHIP_INIT,
			WIRELESS_TX_TYPE_UNKNOWN, di);

	return 0;
}

static int stwlc68_set_rpp_format(u8 pmax, int mode, void *dev_data)
{
	u8 format = 0x80; /* bit[7:6]=10:  Qi RP24 mode No reply */

	format |= ((pmax * STWLC68_RX_RPP_VAL_UNIT) & STWLC68_RX_RPP_VAL_MASK);
	hwlog_info("[%s] format=0x%x\n", __func__, format);
	return stwlc68_write_byte(STWLC68_RX_RPP_SET_ADDR, format);
}

static struct qi_protocol_ops stwlc68_qi_protocol_ops = {
	.chip_name = "stwlc68",
	.send_msg = stwlc68_send_msg,
	.send_msg_with_ack = stwlc68_send_msg_ack,
	.receive_msg = stwlc68_receive_msg,
	.send_fsk_msg = stwlc68_send_fsk_msg,
	.get_ask_packet = stwlc68_get_ask_pkt,
	.get_chip_fw_version = stwlc68_get_chip_fw_version,
	.get_tx_id_pre = stwlc68_get_tx_id_pre,
	.set_rpp_format_post = stwlc68_set_rpp_format,
};

static int stwlc68_ops_register(struct stwlc68_dev_info *di)
{
	int ret;

	ret = stwlc68_fw_ops_register(di);
	if (ret) {
		hwlog_err("ops_register: register fw_ops failed\n");
		return ret;
	}

	g_stwlc68_rx_ic_ops.dev_data = (void *)di;
	ret = wlrx_ic_ops_register(&g_stwlc68_rx_ic_ops, di->ic_type);
	if (ret)
		return ret;

	g_stwlc68_tx_ic_ops.dev_data = (void *)di;
	ret = wltx_ic_ops_register(&g_stwlc68_tx_ic_ops, di->ic_type);
	if (ret) {
		hwlog_err("ops_register: register tx_ic_ops failed\n");
		return ret;
	}

	stwlc68_qi_protocol_ops.dev_data = (void *)g_stwlc68_di;
	ret = qi_protocol_ops_register(&stwlc68_qi_protocol_ops, di->ic_type);
	if (ret) {
		hwlog_err("ops_register: register qi_ops failed\n");
		return ret;
	}
	g_stwlc68_handle = qi_protocol_get_handle();

	return 0;
}

static void stwlc68_shutdown(struct i2c_client *client)
{
	struct stwlc68_dev_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	hwlog_info("[%s] ++\n", __func__);
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON) {
		wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
		msleep(50); /* dalay 50ms for power off */
		(void)stwlc68_set_vfc(ADAPTER_5V * WL_MVOLT_PER_VOLT, di);
		(void)stwlc68_set_rx_vout(ADAPTER_5V * WL_MVOLT_PER_VOLT, di);
		stwlc68_chip_enable(false, di);
		msleep(STWLC68_SHUTDOWN_SLEEP_TIME);
		stwlc68_chip_enable(true, di);
	}
	hwlog_info("[%s] --\n", __func__);
}

static int stwlc68_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	u16 chip_id = 0;
	struct stwlc68_dev_info *di = NULL;
	struct device_node *np = NULL;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!client || !id || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;
	di->ic_type = id->driver_data;
	g_stwlc68_di = di;
	i2c_set_clientdata(client, di);

	ret = stwlc68_dev_check(di);
	if (ret)
		goto dev_ck_fail;

	ret = stwlc68_parse_dts(np, di);
	if (ret)
		goto parse_dts_fail;

	ret = stwlc68_gpio_init(di, np);
	if (ret)
		goto gpio_init_fail;
	ret = stwlc68_irq_init(di, np);
	if (ret)
		goto irq_init_fail;

	wakeup_source_init(&g_stwlc68_wakelock, "stwlc68_wakelock");
	mutex_init(&di->mutex_irq);

	ret = stwlc68_ops_register(di);
	if (ret)
		goto ops_regist_fail;

	if (stwlc68_is_tx_exist(di)) {
		stwlc68_clear_interrupt(STWLC68_ALL_INTR_CLR);
		hwlog_info("[%s] rx exsit, exe rx_ready_handler\n", __func__);
		stwlc68_rx_ready_handler(di);
	} else {
		gpio_set_value(di->gpio_sleep_en, RX_SLEEP_EN_ENABLE);
	}

	stwlc68_get_chip_id(&chip_id);
	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->dev->driver->name;
		power_dev_info->dev_id = chip_id;
		power_dev_info->ver_id = 0;
	}
	if (!power_cmdline_is_powerdown_charging_mode()) {
		INIT_DELAYED_WORK(&di->sram_scan_work, stwlc68_fw_sram_scan_work);
		schedule_delayed_work(&di->sram_scan_work,
			msecs_to_jiffies(1500)); /* delay for wireless probe */
	} else {
		di->g_val.sram_chk_complete = true;
	}

	hwlog_info("wireless_stwlc68 probe ok\n");
	return 0;

ops_regist_fail:
	gpio_free(di->gpio_int);
	free_irq(di->irq_int, di);
irq_init_fail:
	gpio_free(di->gpio_en);
	gpio_free(di->gpio_sleep_en);
	if (di->gpio_pwr_good > 0)
		gpio_free(di->gpio_pwr_good);
gpio_init_fail:
parse_dts_fail:
dev_ck_fail:
	devm_kfree(&client->dev, di);
	di = NULL;
	g_stwlc68_di = NULL;
	np = NULL;
	return ret;
}

MODULE_DEVICE_TABLE(i2c, wireless_stwlc68);
static const struct of_device_id stwlc68_of_match[] = {
	{
		.compatible = "st,stwlc68",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id stwlc68_i2c_id[] = {
	{ "stwlc68", WLTRX_IC_MAIN }, {}
};

static struct i2c_driver stwlc68_driver = {
	.probe = stwlc68_probe,
	.shutdown = stwlc68_shutdown,
	.id_table = stwlc68_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_stwlc68",
		.of_match_table = of_match_ptr(stwlc68_of_match),
	},
};

static int __init stwlc68_init(void)
{
	return i2c_add_driver(&stwlc68_driver);
}

static void __exit stwlc68_exit(void)
{
	i2c_del_driver(&stwlc68_driver);
}

device_initcall(stwlc68_init);
module_exit(stwlc68_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("stwlc68 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
