/*
 * stwlc88.c
 *
 * stwlc88 driver
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

#include "stwlc88.h"

#define HWLOG_TAG wireless_stwlc88
HWLOG_REGIST();

static struct stwlc88_dev_info *g_stwlc88_di;
static struct wakeup_source g_stwlc88_wakelock;

void stwlc88_get_dev_info(struct stwlc88_dev_info **di)
{
	if (!g_stwlc88_di || !di)
		return;

	*di = g_stwlc88_di;
}

bool stwlc88_is_pwr_good(void)
{
	int gpio_val;
	struct stwlc88_dev_info *di = NULL;

	stwlc88_get_dev_info(&di);
	if (!di)
		return false;

	if (!di->g_val.ftp_chk_complete)
		return true;

	gpio_val = gpio_get_value(di->gpio_pwr_good);
	if (gpio_val == ST88_GPIO_PWR_GOOD_VAL)
		return true;

	return false;
}

static int stwlc88_i2c_read(struct i2c_client *client,
	u8 *cmd, int cmd_len, u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!stwlc88_is_pwr_good())
			return -WLC_ERR_I2C_R;
		if (!power_i2c_read_block(client, cmd, cmd_len, buf, buf_len))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -WLC_ERR_I2C_R;
}

static int stwlc88_i2c_write(struct i2c_client *client, u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!stwlc88_is_pwr_good())
			return -WLC_ERR_I2C_W;
		if (!power_i2c_write_block(client, buf, buf_len))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -WLC_ERR_I2C_W;
}

int stwlc88_read_block(u16 reg, u8 *data, u8 len)
{
	u8 cmd[ST88_ADDR_LEN];
	struct stwlc88_dev_info *di = NULL;

	stwlc88_get_dev_info(&di);
	if (!di || !data) {
		hwlog_err("read_block: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;

	return stwlc88_i2c_read(di->client, cmd, ST88_ADDR_LEN, data, len);
}

int stwlc88_write_block(u16 reg, u8 *data, u8 data_len)
{
	int ret;
	u8 *cmd = NULL;
	struct stwlc88_dev_info *di = NULL;

	stwlc88_get_dev_info(&di);
	if (!di || !data) {
		hwlog_err("write_block: para null\n");
		return -WLC_ERR_PARA_NULL;
	}
	cmd = kzalloc(sizeof(u8) * (ST88_ADDR_LEN + data_len), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	memcpy(&cmd[ST88_ADDR_LEN], data, data_len);

	ret = stwlc88_i2c_write(di->client, cmd, ST88_ADDR_LEN + data_len);
	kfree(cmd);
	return ret;
}

int stwlc88_hw_read_block(u32 addr, u8 *data, u8 len)
{
	u8 cmd[ST88_HW_ADDR_F_LEN];
	struct stwlc88_dev_info *di = NULL;

	stwlc88_get_dev_info(&di);
	if (!di || !data) {
		hwlog_err("4addr_read_block: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	 /* bit[0]: flag 0xFA; bit[1:4]: addr */
	cmd[0] = ST88_HW_ADDR_FLAG;
	cmd[1] = (u8)((addr >> 24) & BYTE_MASK);
	cmd[2] = (u8)((addr >> 16) & BYTE_MASK);
	cmd[3] = (u8)((addr >> 8) & BYTE_MASK);
	cmd[4] = (u8)((addr >> 0) & BYTE_MASK);

	return stwlc88_i2c_read(di->client, cmd,
		ST88_HW_ADDR_F_LEN, data, len);
}

int stwlc88_hw_write_block(u32 addr, u8 *data, u8 data_len)
{
	int ret;
	u8 *cmd = NULL;
	struct stwlc88_dev_info *di = NULL;

	stwlc88_get_dev_info(&di);
	if (!di || !data) {
		hwlog_err("4addr_write_block: para null\n");
		return -WLC_ERR_PARA_NULL;
	}
	cmd = kzalloc(sizeof(u8) * (ST88_HW_ADDR_F_LEN + data_len), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	 /* bit[0]: flag 0xFA; bit[1:4]: addr */
	cmd[0] = ST88_HW_ADDR_FLAG;
	cmd[1] = (u8)((addr >> 24) & BYTE_MASK);
	cmd[2] = (u8)((addr >> 16) & BYTE_MASK);
	cmd[3] = (u8)((addr >> 8) & BYTE_MASK);
	cmd[4] = (u8)((addr >> 0) & BYTE_MASK);
	memcpy(&cmd[ST88_HW_ADDR_F_LEN], data, data_len);

	ret = stwlc88_i2c_write(di->client, cmd, ST88_HW_ADDR_F_LEN + data_len);
	kfree(cmd);
	return ret;
}

int stwlc88_read_byte(u16 reg, u8 *data)
{
	return stwlc88_read_block(reg, data, BYTE_LEN);
}

int stwlc88_read_word(u16 reg, u16 *data)
{
	u8 buff[WORD_LEN] = { 0 };

	if (!data || stwlc88_read_block(reg, buff, WORD_LEN))
		return -WLC_ERR_I2C_R;

	*data = buff[0] | buff[1] << BITS_PER_BYTE;
	return 0;
}

int stwlc88_write_byte(u16 reg, u8 data)
{
	return stwlc88_write_block(reg, &data, BYTE_LEN);
}

int stwlc88_write_word(u16 reg, u16 data)
{
	u8 buff[WORD_LEN];

	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return stwlc88_write_block(reg, buff, WORD_LEN);
}

int stwlc88_read_byte_mask(u16 reg, u8 mask, u8 shift, u8 *data)
{
	int ret;
	u8 val = 0;

	ret = stwlc88_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= mask;
	val >>= shift;
	*data = val;

	return 0;
}

int stwlc88_write_byte_mask(u16 reg, u8 mask, u8 shift, u8 data)
{
	int ret;
	u8 val = 0;

	ret = stwlc88_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return stwlc88_write_byte(reg, val);
}

int stwlc88_read_word_mask(u16 reg, u16 mask, u16 shift, u16 *data)
{
	int ret;
	u16 val = 0;

	ret = stwlc88_read_word(reg, &val);
	if (ret)
		return ret;

	val &= mask;
	val >>= shift;
	*data = val;

	return 0;
}

int stwlc88_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data)
{
	int ret;
	u16 val = 0;

	ret = stwlc88_read_word(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return stwlc88_write_word(reg, val);
}

/*
 * stwlc88 chip_info
 */

int stwlc88_get_chip_id(u16 *chip_id)
{
	return stwlc88_read_word(ST88_CHIP_ID_ADDR, chip_id);
}

int stwlc88_get_ftp_patch_id(u16 *ftp_patch_id)
{
	return stwlc88_read_word(ST88_FTP_PATCH_ID_ADDR, ftp_patch_id);
}

int stwlc88_get_cfg_id(u16 *cfg_id)
{
	return stwlc88_read_word(ST88_CFG_ID_ADDR, cfg_id);
}

static void stlwc88_init_chip_info(struct stwlc88_chip_info *info)
{
	info->chip_id      = 0;
	info->chip_rev     = 0;
	info->cust_id      = 0;
	info->rom_id       = 0;
	info->ftp_patch_id = 0;
	info->ram_patch_id = 0;
	info->cfg_id       = 0;
	info->pe_id        = 0;
}

static int stwlc88_get_chip_info(struct stwlc88_chip_info *info)
{
	int ret;
	u8 chip_info[ST88_CHIP_INFO_LEN] = { 0 };

	stlwc88_init_chip_info(info);
	ret = stwlc88_read_block(ST88_CHIP_INFO_ADDR,
		chip_info, ST88_CHIP_INFO_LEN);
	if (ret)
		return ret;

	/*
	 * addr[0:1]:   chip unique id;
	 * addr[2:2]:   chip revision number;
	 * addr[3:3]:   customer id;
	 * addr[4:5]:   rom id;
	 * addr[6:7]:   ftp_patch id;
	 * addr[8:9]:   ram_patch id;
	 * addr[10:11]: cfg id;
	 * addr[12:13]: pe id;
	 * 1byte = 8bit
	 */
	info->chip_id      = (u16)(chip_info[0] | (chip_info[1] << 8));
	info->chip_rev     = chip_info[2];
	info->cust_id      = chip_info[3];
	info->rom_id       = (u16)(chip_info[4] | (chip_info[5] << 8));
	info->ftp_patch_id = (u16)(chip_info[6] | (chip_info[7] << 8));
	info->ram_patch_id = (u16)(chip_info[8] | (chip_info[9] << 8));
	info->cfg_id       = (u16)(chip_info[10] | (chip_info[11] << 8));
	info->pe_id        = (u16)(chip_info[12] | (chip_info[13] << 8));

	return 0;
}

int stwlc88_get_chip_info_str(char *info_str, int len, void *dev_data)
{
	int ret;
	struct stwlc88_chip_info chip_info;

	if (!info_str || (len < WLTRX_IC_CHIP_INFO_LEN))
		return -EINVAL;

	ret = stwlc88_get_chip_info(&chip_info);
	if (ret)
		return -EIO;

	return snprintf(info_str, WLTRX_IC_CHIP_INFO_LEN,
		"chip_id:stwlc%d ftp_patch_id:0x%x cfg_id:0x%x ram_id:0x%x",
		chip_info.chip_id, chip_info.ftp_patch_id, chip_info.cfg_id,
		chip_info.ram_patch_id);
}

int stwlc88_get_chip_fw_version(u8 *data, int len, void *dev_data)
{
	struct stwlc88_chip_info chip_info;

	/* fw version length must be 4 */
	if (!data || (len != 4)) {
		hwlog_err("get_chip_fw_version: para err\n");
		return -WLC_ERR_PARA_WRONG;
	}

	if (stwlc88_get_chip_info(&chip_info)) {
		hwlog_err("get_chip_fw_version: get chip info failed\n");
		return -WLC_ERR_I2C_R;
	}

	/* byte[0:1]=patch_id, byte[2:3]=cfg_id */
	data[0] = (u8)((chip_info.ftp_patch_id >> 0) & BYTE_MASK);
	data[1] = (u8)((chip_info.ftp_patch_id >> BITS_PER_BYTE) & BYTE_MASK);
	data[2] = (u8)((chip_info.cfg_id >> 0) & BYTE_MASK);
	data[3] = (u8)((chip_info.cfg_id >> BITS_PER_BYTE) & BYTE_MASK);

	return 0;
}

int stwlc88_get_mode(u8 *mode)
{
	int ret;

	if (!mode)
		return -WLC_ERR_PARA_NULL;

	ret = stwlc88_read_byte(ST88_OP_MODE_ADDR, mode);
	if (ret) {
		hwlog_err("get_mode: failed\n");
		return -WLC_ERR_I2C_R;
	}

	return 0;
}

static void stwlc88_wake_lock(void)
{
	if (g_stwlc88_wakelock.active) {
		hwlog_info("[wake_lock] already locked\n");
		return;
	}

	__pm_stay_awake(&g_stwlc88_wakelock);
	hwlog_info("wake_lock\n");
}

static void stwlc88_wake_unlock(void)
{
	if (!g_stwlc88_wakelock.active) {
		hwlog_info("[wake_unlock] already unlocked\n");
		return;
	}

	__pm_relax(&g_stwlc88_wakelock);
	hwlog_info("wake_unlock\n");
}

void stwlc88_enable_irq(struct stwlc88_dev_info *di)
{
	if (!di)
		return;

	mutex_lock(&di->mutex_irq);
	if (!di->irq_active) {
		hwlog_info("[enable_irq] ++\n");
		enable_irq(di->irq_int);
		di->irq_active = true;
	}
	hwlog_info("[enable_irq] --\n");
	mutex_unlock(&di->mutex_irq);
}

void stwlc88_disable_irq_nosync(struct stwlc88_dev_info *di)
{
	if (!di)
		return;

	mutex_lock(&di->mutex_irq);
	if (di->irq_active) {
		hwlog_info("[disable_irq_nosync] ++\n");
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
	}
	hwlog_info("[disable_irq_nosync] --\n");
	mutex_unlock(&di->mutex_irq);
}

void stwlc88_chip_enable(bool enable, void *dev_data)
{
	int gpio_val;
	struct stwlc88_dev_info *di = dev_data;

	if (!di)
		return;

	gpio_set_value(di->gpio_en,
		enable ? di->gpio_en_valid_val : !di->gpio_en_valid_val);
	gpio_val = gpio_get_value(di->gpio_en);
	hwlog_info("[chip_enable] gpio %s now\n", gpio_val ? "high" : "low");
}

bool stwlc88_is_chip_enable(void *dev_data)
{
	int gpio_val;
	struct stwlc88_dev_info *di = dev_data;

	if (!di)
		return false;

	gpio_val = gpio_get_value(di->gpio_en);
	return gpio_val == di->gpio_en_valid_val;
}

void stwlc88_chip_reset(void *dev_data)
{
	int ret;
	u8 data = ST88_RST_SYS;

	ret = stwlc88_hw_write_block(ST88_RST_ADDR, &data, BYTE_LEN);
	if (ret)
		hwlog_info("[chip_reset] ignore i2c failure\n");

	hwlog_info("[chip_reset] succ\n");
}

static void stwlc88_irq_work(struct work_struct *work)
{
	int ret;
	int gpio_val;
	u8 mode = 0;
	struct stwlc88_dev_info *di =
		container_of(work, struct stwlc88_dev_info, irq_work);

	if (!di)
		goto exit;

	gpio_val = gpio_get_value(di->gpio_en);
	if (gpio_val != di->gpio_en_valid_val) {
		hwlog_err("[irq_work] gpio %s\n", gpio_val ? "high" : "low");
		goto exit;
	}
	/* get System Operating Mode */
	ret = stwlc88_get_mode(&mode);
	if (!ret)
		hwlog_info("[irq_work] mode=0x%x\n", mode);
	else
		stwlc88_rx_abnormal_irq_handler(di);

	/* handler irq */
	if ((mode == ST88_OP_MODE_TX) || (mode == ST88_OP_MODE_SA))
		stwlc88_tx_mode_irq_handler(di);
	else if (mode == ST88_OP_MODE_RX)
		stwlc88_rx_mode_irq_handler(di);

exit:
	if (di && !di->g_val.irq_abnormal_flag)
		stwlc88_enable_irq(di);

	stwlc88_wake_unlock();
}

static irqreturn_t stwlc88_interrupt(int irq, void *_di)
{
	struct stwlc88_dev_info *di = _di;

	if (!di) {
		hwlog_err("interrupt: di null\n");
		return IRQ_HANDLED;
	}

	stwlc88_wake_lock();
	hwlog_info("[interrupt] ++\n");
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("interrupt: irq is not enable\n");
		stwlc88_wake_unlock();
	}
	hwlog_info("[interrupt] --\n");

	return IRQ_HANDLED;
}

static int stwlc88_dev_check(struct stwlc88_dev_info *di)
{
	int ret;
	u16 chip_id = 0;

	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	power_usleep(DT_USLEEP_10MS);
	ret = stwlc88_get_chip_id(&chip_id);
	if (ret) {
		hwlog_err("dev_check: failed\n");
		wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
		return ret;
	}
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);

	hwlog_info("[dev_check] chip_id=%d\n", chip_id);
	if ((chip_id == ST88_CHIP_ID) || (chip_id == ST88_CHIP_ID_AB))
		return 0;

	hwlog_err("dev_check: rx_chip not match\n");
	return -WLC_ERR_MISMATCH;
}

struct device_node *stwlc88_dts_dev_node(void *dev_data)
{
	struct stwlc88_dev_info *di = dev_data;

	if (!di || !di->dev)
		return NULL;

	return di->dev->of_node;
}

static int stwlc88_gpio_init(struct stwlc88_dev_info *di,
	struct device_node *np)
{
	/* gpio_en */
	if (power_gpio_config_output(np, "gpio_en", "stwlc88_en",
		&di->gpio_en, di->gpio_en_valid_val))
		goto gpio_en_fail;

	/* gpio_sleep_en */
	if (power_gpio_config_output(np, "gpio_sleep_en", "stwlc88_sleep_en",
		&di->gpio_sleep_en, RX_SLEEP_EN_DISABLE))
		goto gpio_sleep_en_fail;

	/* gpio_pwr_good */
	if (power_gpio_config_input(np, "gpio_pwr_good", "stwlc88_pwr_good",
		&di->gpio_pwr_good))
		goto gpio_pwr_good_fail;

	return 0;

gpio_pwr_good_fail:
	gpio_free(di->gpio_sleep_en);
gpio_sleep_en_fail:
	gpio_free(di->gpio_en);
gpio_en_fail:
	return -EINVAL;
}

static int stwlc88_irq_init(struct stwlc88_dev_info *di,
	struct device_node *np)
{
	if (power_gpio_config_interrupt(np, "gpio_int", "stwlc88_int",
		&di->gpio_int, &di->irq_int))
		goto irq_init_fail_0;

	if (request_irq(di->irq_int, stwlc88_interrupt,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "stwlc88_irq", di)) {
		hwlog_err("irq_init: request stwlc88_irq failed\n");
		goto irq_init_fail_1;
	}

	enable_irq_wake(di->irq_int);
	di->irq_active = true;
	INIT_WORK(&di->irq_work, stwlc88_irq_work);

	return 0;

irq_init_fail_1:
	gpio_free(di->gpio_int);
irq_init_fail_0:
	return -EINVAL;
}

static void stwlc88_register_pwr_dev_info(struct stwlc88_dev_info *di)
{
	int ret;
	u16 chip_id = 0;
	struct power_devices_info_data *pwr_dev_info = NULL;

	ret = stwlc88_get_chip_id(&chip_id);
	if (ret)
		return;

	pwr_dev_info = power_devices_info_register();
	if (pwr_dev_info) {
		pwr_dev_info->dev_name = di->dev->driver->name;
		pwr_dev_info->dev_id = chip_id;
		pwr_dev_info->ver_id = 0;
	}
}

static int stwlc88_ops_register(struct stwlc88_dev_info *di)
{
	int ret;

	ret = stwlc88_fw_ops_register(di);
	if (ret) {
		hwlog_err("ops_register: register fw_ops failed\n");
		return ret;
	}
	ret = stwlc88_rx_ops_register(di);
	if (ret) {
		hwlog_err("ops_register: register rx_ops failed\n");
		return ret;
	}
	ret = stwlc88_tx_ops_register(di);
	if (ret) {
		hwlog_err("ops_register: register tx_ops failed\n");
		return ret;
	}
	ret = stwlc88_qi_ops_register(di);
	if (ret) {
		hwlog_err("ops_register: register qi_ops failed\n");
		return ret;
	}
	di->g_val.qi_hdl = qi_protocol_get_handle();

	ret = stwlc88_hw_test_ops_register();
	if (ret) {
		hwlog_err("ops_register: register hw_test_ops failed\n");
		return ret;
	}

	return 0;
}

static void stwlc88_fw_ftp_check(struct stwlc88_dev_info *di)
{
	if (power_cmdline_is_powerdown_charging_mode() ||
		(!power_cmdline_is_factory_mode() && stwlc88_rx_is_tx_exist(di))) {
		di->g_val.ftp_chk_complete = true;
		return;
	}

	INIT_DELAYED_WORK(&di->ftp_check_work, stwlc88_fw_ftp_check_work);
	schedule_delayed_work(&di->ftp_check_work,
		msecs_to_jiffies(WIRELESS_FW_WORK_DELAYED_TIME));
}

static int stwlc88_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct stwlc88_dev_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !id || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;
	di->ic_type = id->driver_data;
	g_stwlc88_di = di;
	i2c_set_clientdata(client, di);

	ret = stwlc88_dev_check(di);
	if (ret)
		goto dev_ck_fail;

	ret = stwlc88_parse_dts(np, di);
	if (ret)
		goto parse_dts_fail;

	ret = stwlc88_gpio_init(di, np);
	if (ret)
		goto gpio_init_fail;
	ret = stwlc88_irq_init(di, np);
	if (ret)
		goto irq_init_fail;

	wakeup_source_init(&g_stwlc88_wakelock, "stwlc88_wakelock");
	mutex_init(&di->mutex_irq);

	ret = stwlc88_ops_register(di);
	if (ret)
		goto ops_regist_fail;

	stwlc88_fw_ftp_check(di);
	stwlc88_rx_probe_check_tx_exist(di);
	stwlc88_register_pwr_dev_info(di);

	hwlog_info("wireless_chip probe ok\n");
	return 0;

ops_regist_fail:
	gpio_free(di->gpio_int);
	free_irq(di->irq_int, di);
irq_init_fail:
	gpio_free(di->gpio_en);
	gpio_free(di->gpio_sleep_en);
	gpio_free(di->gpio_pwr_good);
gpio_init_fail:
parse_dts_fail:
dev_ck_fail:
	devm_kfree(&client->dev, di);
	g_stwlc88_di = NULL;
	return ret;
}

static void stwlc88_shutdown(struct i2c_client *client)
{
	struct stwlc88_dev_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	hwlog_info("[shutdown] ++\n");
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON)
		stwlc88_rx_shutdown_handler(di);
	hwlog_info("[shutdown] --\n");
}

MODULE_DEVICE_TABLE(i2c, wireless_stwlc88);
static const struct of_device_id stwlc88_of_match[] = {
	{
		.compatible = "st,stwlc88",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id stwlc88_i2c_id[] = {
	{ "stwlc88", WLTRX_IC_MAIN }, {}
};

static struct i2c_driver stwlc88_driver = {
	.probe = stwlc88_probe,
	.shutdown = stwlc88_shutdown,
	.id_table = stwlc88_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_stwlc88",
		.of_match_table = of_match_ptr(stwlc88_of_match),
	},
};

static int __init stwlc88_init(void)
{
	return i2c_add_driver(&stwlc88_driver);
}

static void __exit stwlc88_exit(void)
{
	i2c_del_driver(&stwlc88_driver);
}

device_initcall(stwlc88_init);
module_exit(stwlc88_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("stwlc88 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
