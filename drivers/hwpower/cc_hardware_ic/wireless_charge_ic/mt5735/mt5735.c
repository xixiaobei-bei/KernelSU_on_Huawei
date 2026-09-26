/*
 * mt5735.c
 *
 * mt5735 driver
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

#define HWLOG_TAG wireless_mt5735
HWLOG_REGIST();

static struct mt5735_dev_info *g_mt5735_di;
static struct wakeup_source g_mt5735_wakelock;

void mt5735_get_dev_info(struct mt5735_dev_info **di)
{
	if (!g_mt5735_di || !di)
		return;

	*di = g_mt5735_di;
}

bool mt5735_is_pwr_good(void)
{
	int gpio_val;
	struct mt5735_dev_info *di = NULL;

	mt5735_get_dev_info(&di);
	if (!di)
		return false;

	if (!di->g_val.mtp_chk_complete)
		return true;

	gpio_val = gpio_get_value(di->gpio_pwr_good);
	if (gpio_val == MT5735_GPIO_PWR_GOOD_VAL)
		return true;

	return false;
}

static int mt5735_i2c_read(struct i2c_client *client,
	u8 *cmd, int cmd_len, u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!mt5735_is_pwr_good())
			return -WLC_ERR_I2C_R;
		if (!power_i2c_read_block(client, cmd, cmd_len, buf, buf_len))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -WLC_ERR_I2C_R;
}

static int mt5735_i2c_write(struct i2c_client *client, u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!mt5735_is_pwr_good())
			return -WLC_ERR_I2C_W;
		if (!power_i2c_write_block(client, buf, buf_len))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -WLC_ERR_I2C_W;
}

int mt5735_read_block(u16 reg, u8 *data, u8 len)
{
	u8 cmd[MT5735_ADDR_LEN];
	struct mt5735_dev_info *di = NULL;

	mt5735_get_dev_info(&di);
	if (!di || !data)
		return -WLC_ERR_PARA_NULL;

	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;

	return mt5735_i2c_read(di->client, cmd, MT5735_ADDR_LEN, data, len);
}

int mt5735_write_block(u16 reg, u8 *data, u8 data_len)
{
	int ret;
	u8 *cmd = NULL;
	struct mt5735_dev_info *di = NULL;

	mt5735_get_dev_info(&di);
	if (!di || !data)
		return -WLC_ERR_PARA_NULL;

	cmd = kzalloc(sizeof(u8) * (MT5735_ADDR_LEN + data_len), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	memcpy(&cmd[MT5735_ADDR_LEN], data, data_len);

	ret = mt5735_i2c_write(di->client, cmd, MT5735_ADDR_LEN + data_len);
	kfree(cmd);
	return ret;
}

int mt5735_read_byte(u16 reg, u8 *data)
{
	return mt5735_read_block(reg, data, BYTE_LEN);
}

int mt5735_write_byte(u16 reg, u8 data)
{
	return mt5735_write_block(reg, &data, BYTE_LEN);
}

int mt5735_read_word(u16 reg, u16 *data)
{
	u8 buff[WORD_LEN] = { 0 };

	if (!data || mt5735_read_block(reg, buff, WORD_LEN))
		return -EIO;

	*data = buff[0] | buff[1] << BITS_PER_BYTE;
	return 0;
}

int mt5735_write_word(u16 reg, u16 data)
{
	u8 buff[WORD_LEN];

	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return mt5735_write_block(reg, buff, WORD_LEN);
}

int mt5735_read_dword(u16 reg, u32 *data)
{
	u8 buff[POWER_DWORD_LEN] = { 0 };

	if (!data || mt5735_read_block(reg, buff, POWER_DWORD_LEN))
		return -EIO;

	/* 1dword=4bytes, 1byte=8bit */
	*data = buff[0] | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24);
	return 0;
}

int mt5735_write_dword(u16 reg, u32 data)
{
	u8 buff[POWER_DWORD_LEN];

	/* 1dword=4bytes, 1byte=8bit */
	buff[0] = data & BYTE_MASK;
	buff[1] = data >> 8;
	buff[2] = data >> 16;
	buff[3] = data >> 24;

	return mt5735_write_block(reg, buff, POWER_DWORD_LEN);
}

int mt5735_read_byte_mask(u16 reg, u8 mask, u8 shift, u8 *data)
{
	int ret;
	u8 val = 0;

	ret = mt5735_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= mask;
	val >>= shift;
	*data = val;

	return 0;
}

int mt5735_write_byte_mask(u16 reg, u8 mask, u8 shift, u8 data)
{
	int ret;
	u8 val = 0;

	ret = mt5735_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return mt5735_write_byte(reg, val);
}

int mt5735_read_word_mask(u16 reg, u16 mask, u16 shift, u16 *data)
{
	int ret;
	u16 val = 0;

	ret = mt5735_read_word(reg, &val);
	if (ret)
		return ret;

	val &= mask;
	val >>= shift;
	*data = val;

	return 0;
}

int mt5735_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data)
{
	int ret;
	u16 val = 0;

	ret = mt5735_read_word(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return mt5735_write_word(reg, val);
}

int mt5735_read_dword_mask(u16 reg, u32 mask, u32 shift, u32 *data)
{
	int ret;
	u32 val = 0;

	ret = mt5735_read_dword(reg, &val);
	if (ret)
		return ret;

	val &= mask;
	val >>= shift;
	*data = val;

	return 0;
}

int mt5735_write_dword_mask(u16 reg, u32 mask, u32 shift, u32 data)
{
	int ret;
	u32 val = 0;

	ret = mt5735_read_dword(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return mt5735_write_dword(reg, val);
}

/*
 * mt5735 chip_info
 */

int mt5735_get_hw_chip_id(u16 *chip_id)
{
	return mt5735_read_word(MT5735_HW_CHIP_ID_ADDR, chip_id);
}

int mt5735_get_chip_id(u16 *chip_id)
{
	return mt5735_read_word(MT5735_CHIP_ID_ADDR, chip_id);
}

static int mt5735_get_chip_info(struct mt5735_chip_info *info)
{
	int ret;
	u8 chip_info[MT5735_CHIP_INFO_LEN] = { 0 };

	ret = mt5735_read_block(MT5735_CHIP_INFO_ADDR,
		chip_info, MT5735_CHIP_INFO_LEN);
	if (ret)
		return ret;

	/*
	 * addr[0:1]:   chip unique id
	 * addr[2:2]:   customer id
	 * addr[3:3]:   hw id
	 * addr[4:5]:   minor_ver
	 * addr[6:7]:   major_ver
	 * 1byte equal to 8bit
	 */
	info->chip_id   = (u16)(chip_info[0] | (chip_info[1] << 8));
	info->cust_id   = chip_info[2];
	info->hw_id     = chip_info[3];
	info->minor_ver = (u16)(chip_info[4] | (chip_info[5] << 8));
	info->major_ver = (u16)(chip_info[6] | (chip_info[7] << 8));

	return 0;
}

int mt5735_get_chip_info_str(char *info_str, int len, void *dev_data)
{
	int ret;
	struct mt5735_chip_info chip_info = { 0 };

	if (!info_str || (len < WLTRX_IC_CHIP_INFO_LEN))
		return -EINVAL;

	ret = mt5735_get_chip_info(&chip_info);
	if (ret)
		return -EIO;

	return snprintf(info_str, WLTRX_IC_CHIP_INFO_LEN,
		"chip_id:mt0x%x minor_ver:0x%x major_ver:0x%x",
		chip_info.chip_id, chip_info.minor_ver, chip_info.major_ver);
}

int mt5735_get_chip_fw_version(u8 *data, int len, void *dev_data)
{
	struct mt5735_chip_info chip_info = { 0 };

	/* fw version length must be 4 */
	if (!data || (len != 4)) {
		hwlog_err("get_chip_fw_version: para err\n");
		return -WLC_ERR_PARA_WRONG;
	}

	if (mt5735_get_chip_info(&chip_info)) {
		hwlog_err("get_chip_fw_version: get chip info failed\n");
		return -WLC_ERR_I2C_R;
	}

	/* byte[0:1]=major_fw_ver, byte[2:3]=minor_fw_ver */
	data[0] = (u8)((chip_info.major_ver >> 0) & BYTE_MASK);
	data[1] = (u8)((chip_info.major_ver >> BITS_PER_BYTE) & BYTE_MASK);
	data[2] = (u8)((chip_info.minor_ver >> 0) & BYTE_MASK);
	data[3] = (u8)((chip_info.minor_ver >> BITS_PER_BYTE) & BYTE_MASK);

	return 0;
}

int mt5735_get_mode(u16 *mode)
{
	int ret;

	if (!mode)
		return -WLC_ERR_PARA_NULL;

	ret = mt5735_read_word(MT5735_OP_MODE_ADDR, mode);
	if (ret) {
		hwlog_err("get_mode: failed\n");
		return -WLC_ERR_I2C_R;
	}
	*mode &= MT5735_SYS_MODE_MASK;
	return 0;
}

static void mt5735_wake_lock(void)
{
	if (g_mt5735_wakelock.active) {
		hwlog_info("[wake_lock] already locked\n");
		return;
	}

	__pm_stay_awake(&g_mt5735_wakelock);
	hwlog_info("wake_lock\n");
}

static void mt5735_wake_unlock(void)
{
	if (!g_mt5735_wakelock.active) {
		hwlog_info("[wake_unlock] already unlocked\n");
		return;
	}

	__pm_relax(&g_mt5735_wakelock);
	hwlog_info("wake_unlock\n");
}

void mt5735_enable_irq(struct mt5735_dev_info *di)
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

void mt5735_disable_irq_nosync(struct mt5735_dev_info *di)
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

void mt5735_chip_enable(bool enable, void *dev_data)
{
	int gpio_val;
	struct mt5735_dev_info *di = dev_data;

	if (!di)
		return;

	gpio_set_value(di->gpio_en,
		enable ? di->gpio_en_valid_val : !di->gpio_en_valid_val);
	gpio_val = gpio_get_value(di->gpio_en);
	hwlog_info("[chip_enable] gpio %s now\n", gpio_val ? "high" : "low");
}

bool mt5735_is_chip_enable(void *dev_data)
{
	int gpio_val;
	struct mt5735_dev_info *di = dev_data;

	if (!di)
		return false;

	gpio_val = gpio_get_value(di->gpio_en);
	return gpio_val == di->gpio_en_valid_val;
}

void mt5735_chip_reset(void *dev_data)
{
	int ret;

	ret = mt5735_write_dword_mask(MT5735_TX_CMD_ADDR, MT5735_TX_CMD_RST_SYS,
		MT5735_TX_CMD_RST_SYS_SHIFT, MT5735_TX_CMD_VAL);
	if (ret)
		hwlog_err("[chip_reset] ignore i2c failure\n");

	hwlog_info("[chip_reset] succ\n");
}

static void mt5735_irq_work(struct work_struct *work)
{
	int ret;
	int gpio_val;
	u16 mode = 0;
	struct mt5735_dev_info *di =
		container_of(work, struct mt5735_dev_info, irq_work);

	if (!di)
		goto exit;

	gpio_val = gpio_get_value(di->gpio_en);
	if (gpio_val != di->gpio_en_valid_val) {
		hwlog_err("[irq_work] gpio %s\n", gpio_val ? "high" : "low");
		goto exit;
	}
	/* get System Operating Mode */
	ret = mt5735_get_mode(&mode);
	if (!ret)
		hwlog_info("[irq_work] mode=0x%x\n", mode);
	else
		mt5735_rx_abnormal_irq_handler(di);

	/* handler irq */
	if ((mode == MT5735_OP_MODE_TX) || (mode == MT5735_OP_MODE_SA))
		mt5735_tx_mode_irq_handler(di);
	else if (mode == MT5735_OP_MODE_RX)
		mt5735_rx_mode_irq_handler(di);

exit:
	if (di && !di->g_val.irq_abnormal_flag)
		mt5735_enable_irq(di);

	mt5735_wake_unlock();
}

static irqreturn_t mt5735_interrupt(int irq, void *_di)
{
	struct mt5735_dev_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	mt5735_wake_lock();
	hwlog_info("[interrupt] ++\n");
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("interrupt: irq is not enable\n");
		mt5735_wake_unlock();
	}
	hwlog_info("[interrupt] --\n");

	return IRQ_HANDLED;
}

static int mt5735_dev_check(struct mt5735_dev_info *di)
{
	int ret;
	u16 chip_id = 0;

	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	power_usleep(DT_USLEEP_10MS);
	ret = mt5735_get_hw_chip_id(&chip_id);
	if (ret) {
		hwlog_err("dev_check: failed\n");
		wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
		return ret;
	}
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);

	hwlog_info("[dev_check] chip_id=0x%x\n", chip_id);
	if ((chip_id == MT5735_HW_CHIP_ID) || (chip_id == MT5735_CHIP_ID_AB))
		return 0;

	hwlog_err("dev_check: rx_chip not match\n");
	return -WLC_ERR_MISMATCH;
}

struct device_node *mt5735_dts_dev_node(void *dev_data)
{
	struct mt5735_dev_info *di = dev_data;

	if (!di || !di->dev)
		return NULL;

	return di->dev->of_node;
}

static int mt5735_gpio_init(struct mt5735_dev_info *di,
	struct device_node *np)
{
	/* gpio_en */
	if (power_gpio_config_output(np, "gpio_en", "mt5735_en",
		&di->gpio_en, di->gpio_en_valid_val))
		goto gpio_en_fail;

	/* gpio_sleep_en */
	if (power_gpio_config_output(np, "gpio_sleep_en", "mt5735_sleep_en",
		&di->gpio_sleep_en, RX_SLEEP_EN_DISABLE))
		goto gpio_sleep_en_fail;

	/* gpio_pwr_good */
	if (power_gpio_config_input(np, "gpio_pwr_good", "mt5735_pwr_good",
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

static int mt5735_irq_init(struct mt5735_dev_info *di,
	struct device_node *np)
{
	if (power_gpio_config_interrupt(np, "gpio_int", "mt5735_int",
		&di->gpio_int, &di->irq_int))
		goto irq_init_fail_0;

	if (request_irq(di->irq_int, mt5735_interrupt,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "mt5735_irq", di)) {
		hwlog_err("irq_init: request mt5735_irq failed\n");
		goto irq_init_fail_1;
	}

	enable_irq_wake(di->irq_int);
	di->irq_active = true;
	INIT_WORK(&di->irq_work, mt5735_irq_work);

	return 0;

irq_init_fail_1:
	gpio_free(di->gpio_int);
irq_init_fail_0:
	return -EINVAL;
}

static void mt5735_register_pwr_dev_info(struct mt5735_dev_info *di)
{
	int ret;
	u16 chip_id = 0;
	struct power_devices_info_data *pwr_dev_info = NULL;

	ret = mt5735_get_chip_id(&chip_id);
	if (ret)
		return;

	pwr_dev_info = power_devices_info_register();
	if (pwr_dev_info) {
		pwr_dev_info->dev_name = di->dev->driver->name;
		pwr_dev_info->dev_id = chip_id;
		pwr_dev_info->ver_id = 0;
	}
}

static int mt5735_ops_register(struct mt5735_dev_info *di)
{
	int ret;

	ret = mt5735_fw_ops_register(di);
	if (ret) {
		hwlog_err("ops_register: register fw_ops failed\n");
		return ret;
	}
	ret = mt5735_rx_ops_register(di);
	if (ret) {
		hwlog_err("ops_register: register rx_ops failed\n");
		return ret;
	}
	ret = mt5735_tx_ops_register(di);
	if (ret) {
		hwlog_err("ops_register: register tx_ops failed\n");
		return ret;
	}
	ret = mt5735_qi_ops_register(di);
	if (ret) {
		hwlog_err("ops_register: register qi_ops failed\n");
		return ret;
	}
	di->g_val.qi_hdl = qi_protocol_get_handle();

	ret = mt5735_hw_test_ops_register();
	if (ret) {
		hwlog_err("ops_register: register hw_test_ops failed\n");
		return ret;
	}

	return 0;
}

static void mt5735_fw_mtp_check(struct mt5735_dev_info *di)
{
	if (power_cmdline_is_powerdown_charging_mode() ||
		(!power_cmdline_is_factory_mode() && mt5735_rx_is_tx_exist(di))) {
		di->g_val.mtp_chk_complete = true;
		return;
	}

	INIT_DELAYED_WORK(&di->mtp_check_work, mt5735_fw_mtp_check_work);
	schedule_delayed_work(&di->mtp_check_work,
		msecs_to_jiffies(WIRELESS_FW_WORK_DELAYED_TIME));
}

static int mt5735_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct mt5735_dev_info *di = NULL;
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
	g_mt5735_di = di;
	i2c_set_clientdata(client, di);

	ret = mt5735_dev_check(di);
	if (ret)
		goto dev_ck_fail;

	ret = mt5735_parse_dts(np, di);
	if (ret)
		goto parse_dts_fail;

	ret = mt5735_gpio_init(di, np);
	if (ret)
		goto gpio_init_fail;
	ret = mt5735_irq_init(di, np);
	if (ret)
		goto irq_init_fail;

	wakeup_source_init(&g_mt5735_wakelock, "mt5735_wakelock");
	mutex_init(&di->mutex_irq);

	ret = mt5735_ops_register(di);
	if (ret)
		goto ops_regist_fail;

	mt5735_fw_mtp_check(di);
	mt5735_rx_probe_check_tx_exist(di);
	mt5735_register_pwr_dev_info(di);

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
	g_mt5735_di = NULL;
	return ret;
}

static int mt5735_remove(struct i2c_client *client)
{
	struct mt5735_dev_info *l_dev = i2c_get_clientdata(client);

	if (!l_dev)
		return -ENODEV;

	gpio_free(l_dev->gpio_en);
	gpio_free(l_dev->gpio_sleep_en);
	gpio_free(l_dev->gpio_pwr_good);
	gpio_free(l_dev->gpio_int);
	free_irq(l_dev->irq_int, l_dev);
	mutex_destroy(&l_dev->mutex_irq);
	wakeup_source_trash(&g_mt5735_wakelock);
	cancel_delayed_work(&l_dev->mtp_check_work);
	devm_kfree(&client->dev, l_dev);
	g_mt5735_di = NULL;

	return 0;
}

static void mt5735_shutdown(struct i2c_client *client)
{
	struct mt5735_dev_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	hwlog_info("[shutdown] ++\n");
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON)
		mt5735_rx_shutdown_handler(di);
	hwlog_info("[shutdown] --\n");
}

MODULE_DEVICE_TABLE(i2c, wireless_mt5735);
static const struct of_device_id mt5735_of_match[] = {
	{
		.compatible = "mt,mt5735",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id mt5735_i2c_id[] = {
	{ "mt5735", WLTRX_IC_MAIN }, {}
};

static struct i2c_driver mt5735_driver = {
	.probe = mt5735_probe,
	.shutdown = mt5735_shutdown,
	.remove = mt5735_remove,
	.id_table = mt5735_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_mt5735",
		.of_match_table = of_match_ptr(mt5735_of_match),
	},
};

static int __init mt5735_init(void)
{
	return i2c_add_driver(&mt5735_driver);
}

static void __exit mt5735_exit(void)
{
	i2c_del_driver(&mt5735_driver);
}

device_initcall(mt5735_init);
module_exit(mt5735_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("mt5735 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
