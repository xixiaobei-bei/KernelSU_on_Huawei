/*
 * cps4057.c
 *
 * cps4057 driver
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

#include "cps4057.h"

#define HWLOG_TAG wireless_cps4057
HWLOG_REGIST();

static struct cps4057_dev_info *g_cps4057_di;
static struct wakeup_source g_cps4057_wakelock;

void cps4057_get_dev_info(struct cps4057_dev_info **di)
{
	if (!g_cps4057_di || !di)
		return;

	*di = g_cps4057_di;
}

bool cps4057_is_pwr_good(void)
{
	int gpio_val;
	struct cps4057_dev_info *di = NULL;

	cps4057_get_dev_info(&di);
	if (!di)
		return false;

	if (!di->g_val.mtp_chk_complete)
		return true;

	gpio_val = gpio_get_value(di->gpio_pwr_good);
	if (gpio_val == CPS4057_GPIO_PWR_GOOD_VAL)
		return true;

	return false;
}

static int cps4057_i2c_read(struct i2c_client *client,
	u8 *cmd, int cmd_len, u8 *dat, int dat_len)
{
	int i;
	int ret;

	if (!client || !cmd || !dat) {
		hwlog_err("i2c_read: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!cps4057_is_pwr_good())
			return -WLC_ERR_I2C_R;
		ret = power_i2c_read_block(client, cmd, cmd_len, dat, dat_len);
		if (!ret)
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -WLC_ERR_I2C_R;
}

static int cps4057_i2c_write(struct i2c_client *client, u8 *cmd, int cmd_len)
{
	int i;
	int ret;

	if (!client || !cmd) {
		hwlog_err("i2c_write: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!cps4057_is_pwr_good())
			return -WLC_ERR_I2C_W;
		ret = power_i2c_write_block(client, cmd, cmd_len);
		if (!ret)
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -WLC_ERR_I2C_W;
}

int cps4057_read_block(u16 reg, u8 *data, u8 len)
{
	int ret;
	u8 cmd[CPS4057_ADDR_LEN];
	struct cps4057_dev_info *di = NULL;

	cps4057_get_dev_info(&di);
	if (!di || !data) {
		hwlog_err("read_block: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	di->client->addr = CPS4057_SW_I2C_ADDR;
	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;

	ret = cps4057_i2c_read(di->client, cmd, CPS4057_ADDR_LEN, data, len);
	if (ret)
		return ret;

	return 0;
}

int cps4057_write_block(u16 reg, u8 *data, u8 len)
{
	int ret;
	u8 *cmd = NULL;
	struct cps4057_dev_info *di = NULL;

	cps4057_get_dev_info(&di);
	if (!di || !di->client || !data) {
		hwlog_err("write_block: para null\n");
		return -WLC_ERR_PARA_NULL;
	}
	cmd = kzalloc(sizeof(u8) * (CPS4057_ADDR_LEN + len), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	di->client->addr = CPS4057_SW_I2C_ADDR;
	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	memcpy(&cmd[CPS4057_ADDR_LEN], data, len);

	ret = cps4057_i2c_write(di->client, cmd, CPS4057_ADDR_LEN + len);
	kfree(cmd);
	return ret;
}

int cps4057_read_byte(u16 reg, u8 *data)
{
	return cps4057_read_block(reg, data, BYTE_LEN);
}

int cps4057_read_word(u16 reg, u16 *data)
{
	u8 buff[WORD_LEN] = { 0 };

	if (!data || cps4057_read_block(reg, buff, WORD_LEN))
		return -WLC_ERR_I2C_R;

	*data = buff[0] | (buff[1] << BITS_PER_BYTE);
	return 0;
}

int cps4057_write_byte(u16 reg, u8 data)
{
	return cps4057_write_block(reg, &data, BYTE_LEN);
}

int cps4057_write_word(u16 reg, u16 data)
{
	u8 buff[WORD_LEN];

	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return cps4057_write_block(reg, buff, WORD_LEN);
}

int cps4057_write_byte_mask(u16 reg, u8 mask, u8 shift, u8 data)
{
	int ret;
	u8 val = 0;

	ret = cps4057_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return cps4057_write_byte(reg, val);
}

int cps4057_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data)
{
	int ret;
	u16 val = 0;

	ret = cps4057_read_word(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return cps4057_write_word(reg, val);
}

static int cps4057_aux_write_block(u16 reg, u8 *data, u8 data_len)
{
	int ret;
	u8 *cmd = NULL;
	struct cps4057_dev_info *di = NULL;

	cps4057_get_dev_info(&di);
	if (!di || !data) {
		hwlog_err("write_block: para null\n");
		return -WLC_ERR_PARA_NULL;
	}
	cmd = kzalloc(sizeof(u8) * (CPS4057_ADDR_LEN + data_len), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	di->client->addr = CPS4057_HW_I2C_ADDR;
	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	memcpy(&cmd[CPS4057_ADDR_LEN], data, data_len);

	ret = cps4057_i2c_write(di->client, cmd, CPS4057_ADDR_LEN + data_len);
	kfree(cmd);
	return ret;
}

int cps4057_aux_write_word(u16 reg, u16 data)
{
	u8 buff[WORD_LEN];

	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return cps4057_aux_write_block(reg, buff, WORD_LEN);
}

/*
 * cps4057 chip_info
 */
int cps4057_get_chip_id(u16 *chip_id)
{
	int ret;

	ret = cps4057_read_word(CPS4057_CHIP_ID_ADDR, chip_id);
	if (ret)
		return ret;
	*chip_id = CPS4057_CHIP_ID;
	return 0;
}

static int cps4057_get_mtp_version(u16 *mtp_ver)
{
	return cps4057_read_word(CPS4057_MTP_VER_ADDR, mtp_ver);
}

int cps4057_get_chip_info(struct cps4057_chip_info *info)
{
	int ret;

	if (!info)
		return -WLC_ERR_PARA_NULL;

	ret = cps4057_get_chip_id(&info->chip_id);
	ret += cps4057_get_mtp_version(&info->mtp_ver);
	if (ret) {
		hwlog_err("get_chip_info: failed\n");
		return ret;
	}

	return 0;
}

int cps4057_get_chip_info_str(char *info_str, int len, void *dev_data)
{
	int ret;
	struct cps4057_chip_info info = { 0 };

	if (!info_str || (len < WLTRX_IC_CHIP_INFO_LEN))
		return -EINVAL;

	ret = cps4057_get_chip_info(&info);
	if (ret)
		return -EIO;

	return snprintf(info_str, WLTRX_IC_CHIP_INFO_LEN,
		"chip_id:0x%x mtp_ver:0x%x", info.chip_id, info.mtp_ver);
}

int cps4057_get_mode(u8 *mode)
{
	int ret;

	if (!mode)
		return -WLC_ERR_PARA_NULL;

	ret = cps4057_read_byte(CPS4057_OP_MODE_ADDR, mode);
	if (ret) {
		hwlog_err("get_mode: failed\n");
		return -WLC_ERR_I2C_R;
	}

	return 0;
}

static void cps4057_wake_lock(void)
{
	if (g_cps4057_wakelock.active) {
		hwlog_info("[wake_lock] already locked\n");
		return;
	}

	__pm_stay_awake(&g_cps4057_wakelock);
	hwlog_info("wake_lock\n");
}

static void cps4057_wake_unlock(void)
{
	if (!g_cps4057_wakelock.active) {
		hwlog_info("[wake_unlock] already unlocked\n");
		return;
	}

	__pm_relax(&g_cps4057_wakelock);
	hwlog_info("wake_unlock\n");
}

void cps4057_enable_irq(struct cps4057_dev_info *di)
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

void cps4057_disable_irq_nosync(struct cps4057_dev_info *di)
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

void cps4057_chip_enable(bool enable, void *dev_data)
{
	int gpio_val;
	struct cps4057_dev_info *di = dev_data;

	if (!di)
		return;

	gpio_set_value(di->gpio_en,
		enable ? di->gpio_en_valid_val : !di->gpio_en_valid_val);
	gpio_val = gpio_get_value(di->gpio_en);
	hwlog_info("[chip_enable] gpio %s now\n", gpio_val ? "high" : "low");
}

bool cps4057_is_chip_enable(void *dev_data)
{
	int gpio_val;
	struct cps4057_dev_info *di = dev_data;

	if (!di)
		return false;

	gpio_val = gpio_get_value(di->gpio_en);
	return gpio_val == di->gpio_en_valid_val;
}

static void cps4057_irq_work(struct work_struct *work)
{
	int ret;
	int gpio_val;
	u8 mode = 0;
	struct cps4057_dev_info *di =
		container_of(work, struct cps4057_dev_info, irq_work);

	if (!di)
		goto exit;

	gpio_val = gpio_get_value(di->gpio_en);
	if (gpio_val != di->gpio_en_valid_val) {
		hwlog_err("[irq_work] gpio %s\n", gpio_val ? "high" : "low");
		goto exit;
	}
	/* get System Operating Mode */
	ret = cps4057_get_mode(&mode);
	if (!ret)
		hwlog_info("[irq_work] mode=0x%x\n", mode);
	else
		cps4057_rx_abnormal_irq_handler(di);

	/* handler irq */
	if ((mode == CPS4057_OP_MODE_TX) || (mode == CPS4057_OP_MODE_BP))
		cps4057_tx_mode_irq_handler(di);
	else if (mode == CPS4057_OP_MODE_RX)
		cps4057_rx_mode_irq_handler(di);

exit:
	if (di && !di->g_val.irq_abnormal_flag)
		cps4057_enable_irq(di);

	cps4057_wake_unlock();
}

static irqreturn_t cps4057_interrupt(int irq, void *_di)
{
	struct cps4057_dev_info *di = _di;

	if (!di) {
		hwlog_err("interrupt: di null\n");
		return IRQ_HANDLED;
	}

	cps4057_wake_lock();
	hwlog_info("[interrupt] ++\n");
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("[interrupt] irq is not enable\n");
		cps4057_wake_unlock();
	}
	hwlog_info("[interrupt] --\n");

	return IRQ_HANDLED;
}

static int cps4057_dev_check(struct cps4057_dev_info *di)
{
	int ret;
	u16 chip_id = 0;

	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	power_usleep(DT_USLEEP_10MS);
	ret = cps4057_get_chip_id(&chip_id);
	if (ret) {
		hwlog_err("dev_check: failed\n");
		wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
		return ret;
	}
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);

	hwlog_info("[dev_check] chip_id=0x%04x\n", chip_id);
	if (chip_id != CPS4057_CHIP_ID)
		hwlog_err("dev_check: rx_chip not match\n");

	return 0;
}

struct device_node *cps4057_dts_dev_node(void *dev_data)
{
	struct cps4057_dev_info *di = dev_data;

	if (!di || !di->dev)
		return NULL;

	return di->dev->of_node;
}

static int cps4057_gpio_init(struct cps4057_dev_info *di,
	struct device_node *np)
{
	if (power_gpio_config_output(np, "gpio_en", "cps4057_en",
		&di->gpio_en, di->gpio_en_valid_val))
		goto gpio_en_fail;

	if (power_gpio_config_output(np, "gpio_sleep_en", "cps4057_sleep_en",
		&di->gpio_sleep_en, RX_SLEEP_EN_DISABLE))
		goto gpio_sleep_en_fail;

	if (power_gpio_config_input(np, "gpio_pwr_good", "cps4057_pwr_good",
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

static int cps4057_irq_init(struct cps4057_dev_info *di,
	struct device_node *np)
{
	if (power_gpio_config_interrupt(np, "gpio_int", "cps4057_int",
		&di->gpio_int, &di->irq_int))
		return -EINVAL;

	if (request_irq(di->irq_int, cps4057_interrupt,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "cps4057_irq", di)) {
		hwlog_err("irq_init: request cps4057_irq failed\n");
		gpio_free(di->gpio_int);
		return -EINVAL;
	}

	enable_irq_wake(di->irq_int);
	di->irq_active = true;
	INIT_WORK(&di->irq_work, cps4057_irq_work);

	return 0;
}

static void cps4057_register_pwr_dev_info(struct cps4057_dev_info *di)
{
	int ret;
	u16 chip_id = 0;
	struct power_devices_info_data *pwr_dev_info = NULL;

	ret = cps4057_get_chip_id(&chip_id);
	if (ret)
		return;

	pwr_dev_info = power_devices_info_register();
	if (pwr_dev_info) {
		pwr_dev_info->dev_name = di->dev->driver->name;
		pwr_dev_info->dev_id = chip_id;
		pwr_dev_info->ver_id = 0;
	}
}

static int cps4057_ops_register(struct cps4057_dev_info *di)
{
	int ret;

	ret = cps4057_fw_ops_register(di);
	if (ret) {
		hwlog_err("ops_register: register fw_ops failed\n");
		return ret;
	}

	ret = cps4057_rx_ops_register(di);
	if (ret) {
		hwlog_err("ops_register: register rx_ops failed\n");
		return ret;
	}

	ret = cps4057_tx_ops_register(di);
	if (ret) {
		hwlog_err("ops_register: register tx_ops failed\n");
		return ret;
	}

	ret = cps4057_qi_ops_register(di);
	if (ret) {
		hwlog_err("ops_register: register qi_ops failed\n");
		return ret;
	}
	di->g_val.qi_hdl = qi_protocol_get_handle();

	ret = cps4057_hw_test_ops_register();
	if (ret) {
		hwlog_err("ops_register: register hw_test_ops failed\n");
		return ret;
	}

	return 0;
}

static void cps4057_fw_mtp_check(struct cps4057_dev_info *di)
{
	if (power_cmdline_is_powerdown_charging_mode() ||
		(!power_cmdline_is_factory_mode() && cps4057_rx_is_tx_exist(di))) {
		di->g_val.mtp_chk_complete = true;
		return;
	}

	INIT_DELAYED_WORK(&di->mtp_check_work, cps4057_fw_mtp_check_work);
	schedule_delayed_work(&di->mtp_check_work,
		msecs_to_jiffies(WIRELESS_FW_WORK_DELAYED_TIME));
}

static int cps4057_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct cps4057_dev_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !id || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->ic_type = id->driver_data;
	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;
	g_cps4057_di = di;
	i2c_set_clientdata(client, di);

	ret = cps4057_dev_check(di);
	if (ret)
		goto dev_ck_fail;

	ret = cps4057_parse_dts(np, di);
	if (ret)
		goto parse_dts_fail;

	ret = cps4057_gpio_init(di, np);
	if (ret)
		goto gpio_init_fail;

	ret = cps4057_irq_init(di, np);
	if (ret)
		goto irq_init_fail;

	wakeup_source_init(&g_cps4057_wakelock, "cps4057_wakelock");
	mutex_init(&di->mutex_irq);

	ret = cps4057_ops_register(di);
	if (ret)
		goto ops_regist_fail;

	cps4057_fw_mtp_check(di);
	cps4057_rx_probe_check_tx_exist(di);
	cps4057_register_pwr_dev_info(di);

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
	g_cps4057_di = NULL;
	return ret;
}

static void cps4057_shutdown(struct i2c_client *client)
{
	struct cps4057_dev_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	hwlog_info("[shutdown] ++\n");
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON)
		cps4057_rx_shutdown_handler(di);
	hwlog_info("[shutdown] --\n");
}

static const struct of_device_id cps4057_of_match[] = {
	{
		.compatible = "cps,cps4057",
	},
	{},
};
MODULE_DEVICE_TABLE(of, cps4057_of_match);

static const struct i2c_device_id cps4057_i2c_id[] = {
	{ "cps4057", WLTRX_IC_MAIN }, {}
};
MODULE_DEVICE_TABLE(i2c, cps4057_i2c_id);

static struct i2c_driver cps4057_driver = {
	.probe = cps4057_probe,
	.shutdown = cps4057_shutdown,
	.id_table = cps4057_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_cps4057",
		.of_match_table = of_match_ptr(cps4057_of_match),
	},
};

static int __init cps4057_init(void)
{
	return i2c_add_driver(&cps4057_driver);
}

static void __exit cps4057_exit(void)
{
	i2c_del_driver(&cps4057_driver);
}

device_initcall(cps4057_init);
module_exit(cps4057_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("cps4057 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
