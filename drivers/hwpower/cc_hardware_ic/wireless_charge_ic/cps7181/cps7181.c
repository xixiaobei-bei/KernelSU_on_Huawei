/*
 * cps7181.c
 *
 * cps7181 driver
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

#include "cps7181.h"

#define HWLOG_TAG wireless_cps7181
HWLOG_REGIST();

static bool cps7181_is_pwr_good(struct cps7181_dev_info *di)
{
	int gpio_val;

	if (!di)
		return false;

	if (di->ic_type == WLTRX_IC_AUX)
		return true;

	if (!di->g_val.mtp_chk_complete)
		return true;

	gpio_val = gpio_get_value(di->gpio_pwr_good);

	return gpio_val == CPS7181_GPIO_PWR_GOOD_VAL;
}

static int cps7181_i2c_read(struct cps7181_dev_info *di,
	u8 *cmd, int cmd_len, u8 *dat, int dat_len)
{
	int i;
	int ret;

	if (!di || !cmd || !dat) {
		hwlog_err("i2c_read: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!cps7181_is_pwr_good(di))
			return -WLC_ERR_I2C_R;
		ret = power_i2c_read_block(di->client, cmd, cmd_len, dat, dat_len);
		if (!ret)
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -WLC_ERR_I2C_R;
}

static int cps7181_i2c_write(struct cps7181_dev_info *di, u8 *cmd, int cmd_len)
{
	int i;
	int ret;

	if (!di || !cmd) {
		hwlog_err("i2c_write: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!cps7181_is_pwr_good(di))
			return -WLC_ERR_I2C_W;
		ret = power_i2c_write_block(di->client, cmd, cmd_len);
		if (!ret)
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -WLC_ERR_I2C_W;
}

int cps7181_read_block(struct cps7181_dev_info *di, u16 reg, u8 *data, u8 len)
{
	int ret;
	u8 cmd[CPS7181_ADDR_LEN];

	if (!di || !data) {
		hwlog_err("read_block: para null\n");
		return -WLC_ERR_PARA_NULL;
	}

	di->client->addr = CPS7181_SW_I2C_ADDR;
	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;

	ret = cps7181_i2c_read(di, cmd, CPS7181_ADDR_LEN, data, len);
	if (ret) {
		di->g_val.sram_i2c_ready = false;
		return ret;
	}

	return 0;
}

int cps7181_write_block(struct cps7181_dev_info *di, u16 reg, u8 *data, u8 len)
{
	int ret;
	u8 *cmd = NULL;

	if (!di || !di->client || !data) {
		hwlog_err("write_block: para null\n");
		return -WLC_ERR_PARA_NULL;
	}
	cmd = kzalloc(sizeof(u8) * (CPS7181_ADDR_LEN + len), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	di->client->addr = CPS7181_SW_I2C_ADDR;
	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	memcpy(&cmd[CPS7181_ADDR_LEN], data, len);

	ret = cps7181_i2c_write(di, cmd, CPS7181_ADDR_LEN + len);
	if (ret)
		di->g_val.sram_i2c_ready = false;

	kfree(cmd);
	return ret;
}

int cps7181_read_byte(struct cps7181_dev_info *di, u16 reg, u8 *data)
{
	return cps7181_read_block(di, reg, data, BYTE_LEN);
}

int cps7181_read_word(struct cps7181_dev_info *di, u16 reg, u16 *data)
{
	u8 buff[WORD_LEN] = { 0 };

	if (!data || cps7181_read_block(di, reg, buff, WORD_LEN))
		return -WLC_ERR_I2C_R;

	*data = buff[0] | (buff[1] << BITS_PER_BYTE);
	return 0;
}

int cps7181_write_byte(struct cps7181_dev_info *di, u16 reg, u8 data)
{
	return cps7181_write_block(di, reg, &data, BYTE_LEN);
}

int cps7181_write_word(struct cps7181_dev_info *di, u16 reg, u16 data)
{
	u8 buff[WORD_LEN];

	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return cps7181_write_block(di, reg, buff, WORD_LEN);
}

int cps7181_write_byte_mask(struct cps7181_dev_info *di, u16 reg, u8 mask, u8 shift, u8 data)
{
	int ret;
	u8 val = 0;

	ret = cps7181_read_byte(di, reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return cps7181_write_byte(di, reg, val);
}

int cps7181_write_word_mask(struct cps7181_dev_info *di, u16 reg,
	u16 mask, u16 shift, u16 data)
{
	int ret;
	u16 val = 0;

	ret = cps7181_read_word(di, reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return cps7181_write_word(di, reg, val);
}

static int cps7181_aux_write_block(struct cps7181_dev_info *di, u16 reg,
	u8 *data, u8 data_len)
{
	int ret;
	u8 *cmd = NULL;

	if (!di || !data) {
		hwlog_err("write_block: para null\n");
		return -WLC_ERR_PARA_NULL;
	}
	cmd = kzalloc(sizeof(u8) * (CPS7181_ADDR_LEN + data_len), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	di->client->addr = CPS7181_HW_I2C_ADDR;
	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	memcpy(&cmd[CPS7181_ADDR_LEN], data, data_len);

	ret = cps7181_i2c_write(di, cmd, CPS7181_ADDR_LEN + data_len);
	kfree(cmd);
	return ret;
}

int cps7181_aux_write_word(struct cps7181_dev_info *di, u16 reg, u16 data)
{
	u8 buff[WORD_LEN];

	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return cps7181_aux_write_block(di, reg, buff, WORD_LEN);
}

/*
 * cps7181 chip_info
 */
int cps7181_get_chip_id(struct cps7181_dev_info *di, u16 *chip_id)
{
	int ret;

	ret = cps7181_read_word(di, CPS7181_CHIP_ID_ADDR, chip_id);
	if (ret)
		return ret;
	*chip_id = CPS7181_CHIP_ID;
	return 0;
}

static int cps7181_get_mtp_version(struct cps7181_dev_info *di, u16 *mtp_ver)
{
	return cps7181_read_word(di, CPS7181_MTP_VER_ADDR, mtp_ver);
}

int cps7181_get_chip_info(struct cps7181_dev_info *di, struct cps7181_chip_info *info)
{
	int ret;

	if (!info || !di)
		return -WLC_ERR_PARA_NULL;

	ret = cps7181_get_chip_id(di, &info->chip_id);
	ret += cps7181_get_mtp_version(di, &info->mtp_ver);
	if (ret) {
		hwlog_err("get_chip_info: failed\n");
		return ret;
	}

	return 0;
}

int cps7181_get_chip_info_str(char *info_str, int len, void *dev_data)
{
	int ret;
	struct cps7181_chip_info info = { 0 };
	struct cps7181_dev_info *di = dev_data;

	if (!info_str || !di || (len < WLTRX_IC_CHIP_INFO_LEN))
		return -EINVAL;

	ret = cps7181_get_chip_info(di, &info);
	if (ret)
		return -EIO;

	return snprintf(info_str, WLTRX_IC_CHIP_INFO_LEN,
		"chip_id:0x%x mtp_ver:0x%04x", info.chip_id, info.mtp_ver);
}

int cps7181_get_mode(struct cps7181_dev_info *di, u8 *mode)
{
	return cps7181_read_byte(di, CPS7181_OP_MODE_ADDR, mode);
}

static void cps7181_wake_lock(struct cps7181_dev_info *di)
{
	if (di->wakelock.active) {
		hwlog_info("[wake_lock] already locked\n");
		return;
	}

	__pm_stay_awake(&di->wakelock);
	hwlog_info("wake_lock\n");
}

static void cps7181_wake_unlock(struct cps7181_dev_info *di)
{
	if (!di->wakelock.active) {
		hwlog_info("[wake_unlock] already unlocked\n");
		return;
	}

	__pm_relax(&di->wakelock);
	hwlog_info("wake_unlock\n");
}

void cps7181_enable_irq(struct cps7181_dev_info *di)
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

void cps7181_disable_irq_nosync(struct cps7181_dev_info *di)
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

void cps7181_chip_enable(bool enable, void *dev_data)
{
	int gpio_val;
	struct cps7181_dev_info *di = dev_data;

	if (!di)
		return;

	gpio_set_value(di->gpio_en,
		enable ? di->gpio_en_valid_val : !di->gpio_en_valid_val);
	gpio_val = gpio_get_value(di->gpio_en);
	hwlog_info("[chip_enable] gpio %s now\n", gpio_val ? "high" : "low");
}

bool cps7181_is_chip_enable(void *dev_data)
{
	int gpio_val;
	struct cps7181_dev_info *di = dev_data;

	if (!di)
		return false;

	gpio_val = gpio_get_value(di->gpio_en);
	return gpio_val == di->gpio_en_valid_val;
}

static void cps7181_irq_work(struct work_struct *work)
{
	int ret;
	int gpio_val;
	u8 mode = 0;
	u16 chip_id = 0;
	struct cps7181_dev_info *di =
		container_of(work, struct cps7181_dev_info, irq_work);

	if (!di)
		goto exit;

	gpio_val = gpio_get_value(di->gpio_en);
	if (gpio_val != di->gpio_en_valid_val) {
		hwlog_err("[irq_work] gpio %s\n", gpio_val ? "high" : "low");
		goto exit;
	}
	/* init i2c */
	ret = cps7181_read_word(di, CPS7181_CHIP_ID_ADDR, &chip_id);
	if (!di->g_val.sram_i2c_ready || (chip_id != CPS7181_CHIP_ID)) {
		ret = cps7181_fw_sram_i2c_init(di, CPS7181_BYTE_INC);
		if (ret) {
			hwlog_err("irq_work: i2c init failed\n");
			goto exit;
		}
	}
	/* get System Operating Mode */
	ret = cps7181_get_mode(di, &mode);
	if (!ret)
		hwlog_info("[irq_work] mode=0x%x\n", mode);
	else
		cps7181_rx_abnormal_irq_handler(di);

	/* handler irq */
	if ((mode == CPS7181_OP_MODE_TX) || (mode == CPS7181_OP_MODE_BP))
		cps7181_tx_mode_irq_handler(di);
	else if (mode == CPS7181_OP_MODE_RX)
		cps7181_rx_mode_irq_handler(di);

exit:
	if (di && !di->g_val.irq_abnormal_flag)
		cps7181_enable_irq(di);

	cps7181_wake_unlock(di);
}

static irqreturn_t cps7181_interrupt(int irq, void *_di)
{
	struct cps7181_dev_info *di = _di;

	if (!di) {
		hwlog_err("interrupt: di null\n");
		return IRQ_HANDLED;
	}

	cps7181_wake_lock(di);
	hwlog_info("[interrupt] ++\n");
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("[interrupt] irq is not enable\n");
		cps7181_wake_unlock(di);
	}
	hwlog_info("[interrupt] --\n");

	return IRQ_HANDLED;
}

static int cps7181_dev_check(struct cps7181_dev_info *di)
{
	int ret;
	u16 chip_id = 0;

	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	power_usleep(DT_USLEEP_10MS);
	ret = cps7181_get_chip_id(di, &chip_id);
	if (ret) {
		hwlog_err("dev_check: failed\n");
		wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
		return ret;
	}
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);

	hwlog_info("[dev_check] ic_type=%d chip_id=0x%04x\n", di->ic_type, chip_id);
	if (chip_id != CPS7181_CHIP_ID)
		hwlog_err("dev_check: rx_chip not match\n");

	return 0;
}

struct device_node *cps7181_dts_dev_node(void *dev_data)
{
	struct cps7181_dev_info *di = dev_data;

	if (!di || !di->dev)
		return NULL;

	return di->dev->of_node;
}

static int cps7181_gpio_init(struct cps7181_dev_info *di,
	struct device_node *np)
{
	if (power_gpio_config_output(np, "gpio_en", "cps7181_en",
		&di->gpio_en, di->gpio_en_valid_val))
		goto gpio_en_fail;

	if (di->ic_type == WLTRX_IC_AUX)
		return 0;

	if (power_gpio_config_output(np, "gpio_sleep_en", "cps7181_sleep_en",
		&di->gpio_sleep_en, RX_SLEEP_EN_DISABLE))
		goto gpio_sleep_en_fail;

	if (power_gpio_config_input(np, "gpio_pwr_good", "cps7181_pwr_good",
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

static int cps7181_irq_init(struct cps7181_dev_info *di,
	struct device_node *np)
{
	if (power_gpio_config_interrupt(np, "gpio_int", "cps7181_int",
		&di->gpio_int, &di->irq_int))
		return -EINVAL;

	if (request_irq(di->irq_int, cps7181_interrupt,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "cps7181_irq", di)) {
		hwlog_err("irq_init: request cps7181_irq failed\n");
		gpio_free(di->gpio_int);
		return -EINVAL;
	}

	enable_irq_wake(di->irq_int);
	di->irq_active = true;
	INIT_WORK(&di->irq_work, cps7181_irq_work);

	return 0;
}

static void cps7181_register_pwr_dev_info(struct cps7181_dev_info *di)
{
	int ret;
	u16 chip_id = 0;
	struct power_devices_info_data *pwr_dev_info = NULL;

	ret = cps7181_get_chip_id(di, &chip_id);
	if (ret)
		return;

	pwr_dev_info = power_devices_info_register();
	if (pwr_dev_info) {
		pwr_dev_info->dev_name = di->dev->driver->name;
		pwr_dev_info->dev_id = chip_id;
		pwr_dev_info->ver_id = 0;
	}
}

static void cps7181_ops_unregister(struct wltrx_ic_ops *ops)
{
	if (!ops)
		return;

	kfree(ops->rx_ops);
	kfree(ops->tx_ops);
	kfree(ops->fw_ops);
	kfree(ops->qi_ops);
	kfree(ops);
}

static int cps7181_ops_register(struct wltrx_ic_ops *ops, struct cps7181_dev_info *di)
{
	int ret;

	ret = cps7181_fw_ops_register(ops, di);
	if (ret) {
		hwlog_err("ops_register: register fw_ops failed\n");
		goto register_fail;
	}

	if (di->ic_type == WLTRX_IC_MAIN) {
		ret = cps7181_rx_ops_register(ops, di);
		if (ret) {
			hwlog_err("ops_register: register rx_ops failed\n");
			goto register_fail;
		}
	}

	ret = cps7181_tx_ops_register(ops, di);
	if (ret) {
		hwlog_err("ops_register: register tx_ops failed\n");
		goto register_fail;
	}

	ret = cps7181_qi_ops_register(ops, di);
	if (ret) {
		hwlog_err("ops_register: register qi_ops failed\n");
		goto register_fail;
	}
	di->g_val.qi_hdl = qi_protocol_get_handle();

	return 0;

register_fail:
	cps7181_ops_unregister(ops);
	return ret;
}

static int cps7181_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	struct cps7181_dev_info *di = NULL;
	struct device_node *np = NULL;
	struct wltrx_ic_ops *ops = NULL;

	if (!client || !id || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;
	ops = kzalloc(sizeof(*ops), GFP_KERNEL);
	if (!ops) {
		devm_kfree(&client->dev, di);
		return -ENOMEM;
	}

	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;
	di->ic_type = id->driver_data;
	i2c_set_clientdata(client, di);

	ret = cps7181_dev_check(di);
	if (ret)
		goto dev_ck_fail;

	ret = cps7181_parse_dts(np, di);
	if (ret)
		goto parse_dts_fail;

	ret = cps7181_gpio_init(di, np);
	if (ret)
		goto gpio_init_fail;

	ret = cps7181_irq_init(di, np);
	if (ret)
		goto irq_init_fail;

	wakeup_source_init(&di->wakelock, "cps7181_wakelock");
	mutex_init(&di->mutex_irq);

	if (!power_cmdline_is_powerdown_charging_mode()) {
		INIT_WORK(&di->mtp_check_work, cps7181_fw_mtp_check_work);
		schedule_work(&di->mtp_check_work);
	} else {
		di->g_val.mtp_chk_complete = true;
	}

	ret = cps7181_ops_register(ops, di);
	if (ret)
		goto ops_regist_fail;

	cps7181_rx_probe_check_tx_exist(di);
	cps7181_register_pwr_dev_info(di);

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
	cps7181_ops_unregister(ops);
	devm_kfree(&client->dev, di);
	return ret;
}

static void cps7181_shutdown(struct i2c_client *client)
{
	struct cps7181_dev_info *di = i2c_get_clientdata(client);

	if (!di)
		return;

	hwlog_info("[shutdown] ++\n");
	if (wlrx_get_wireless_channel_state() == WIRELESS_CHANNEL_ON)
		cps7181_rx_shutdown_handler(di);
	hwlog_info("[shutdown] --\n");
}

MODULE_DEVICE_TABLE(i2c, wireless_cps7181);
static const struct of_device_id cps7181_of_match[] = {
	{
		.compatible = "cps,cps7181",
		.data = NULL,
	},
	{
		.compatible = "cps,cps7181_aux",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id cps7181_i2c_id[] = {
	{ "cps7181", WLTRX_IC_MAIN },
	{ "cps7181_aux", WLTRX_IC_AUX },
	{}
};

static struct i2c_driver cps7181_driver = {
	.probe = cps7181_probe,
	.shutdown = cps7181_shutdown,
	.id_table = cps7181_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_cps7181",
		.of_match_table = of_match_ptr(cps7181_of_match),
	},
};

static int __init cps7181_init(void)
{
	return i2c_add_driver(&cps7181_driver);
}

static void __exit cps7181_exit(void)
{
	i2c_del_driver(&cps7181_driver);
}

device_initcall(cps7181_init);
module_exit(cps7181_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("cps7181 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
