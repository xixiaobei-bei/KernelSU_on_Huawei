/*
 * bq27z561.c
 *
 * coul with bq27z561 driver
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

#include "bq27z561.h"

#define HWLOG_TAG bq27z561
HWLOG_REGIST();

#define BQ27Z561_CALI_MIN_A             800000
#define BQ27Z561_CALI_MAX_A             1300000
#define BQ27Z561_CALI_PRECIE            1000000
#define BQ27Z561_CALI_UNIT              65536
#define BQ27Z561_CALI_CUR_TH            32768
#define BQ27Z561_CALI_CUR_MA            368
#define BQ27Z561_CALI_CUR_BASE          100
#define BQ27Z561_CALI_CUR_MAX           0xFFFF
#define BQ27Z561_CELL_GAIN_DEFAULT      12101
#define BQ27Z561_CAP_GAIN_DEFAULT       4390411
#define BQ27Z561_TBATICAL_MIN_A         752000
#define BQ27Z561_TBATICAL_MAX_A         1246000
#define BQ27Z561_TBATCAL_MIN_A          752000
#define BQ27Z561_TBATCAL_MAX_A          1246000

static const u8 bq27z561_dump_regs[] = {
	0x00, 0x02, 0x04, 0x06,
	0x08, 0x0A, 0x0C, 0x0E,
	0x10, 0x16, 0x18, 0x1A,
	0x1C, 0x1E, 0x20, 0x28,
	0x2A, 0x2C, 0x2E, 0x30,
	0x66, 0x68, 0x6C, 0x6E,
};

static struct bq27z561_device_info *g_bq27z561_dev;

static int bq27z561_read_block(u16 addr, u8 reg, u8 *data, u8 len)
{
	int i;
	int ret;
	struct i2c_msg msg[MSG_LEN];
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di || !di->client || !data) {
		hwlog_err("di or data is null\n");
		return -FG_ERR_PARA_NULL;
	}

	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = &reg;
	msg[0].len = 1; /* 1: reg length */

	msg[1].addr = di->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = data;
	msg[1].len = len;

	if (addr == FG_ROM_MODE_I2C_ADDR_CFG) {
		msg[0].addr = FG_ROM_MODE_I2C_ADDR;
		msg[1].addr = FG_ROM_MODE_I2C_ADDR;
	}

	mutex_lock(&di->rd_mutex);
	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(di->client->adapter, msg, MSG_LEN);
		if (ret >= 0)
			break;

		usleep_range(5000, 5100); /* sleep 5ms */
	}
	mutex_unlock(&di->rd_mutex);

	if (ret < 0) {
		hwlog_err("read_block failed[%x]\n", reg);
		return -FG_ERR_I2C_R;
	}

	return 0;
}

static int bq27z561_read_byte(u8 reg, u8 *data)
{
	/* 1: one byte */
	return bq27z561_read_block(INVALID_REG_ADDR, reg, data, 1);
}

static int bq27z561_read_word(u8 reg, u16 *data)
{
	int ret;
	u8 buff[MSG_LEN] = { 0 };

	/* 2: one word */
	ret = bq27z561_read_block(INVALID_REG_ADDR, reg, buff, 2);
	if (ret)
		return -FG_ERR_I2C_R;
	*data = get_unaligned_le16(buff);

	return 0;
}

static int bq27z561_write_block(u16 addr, u8 reg, u8 *buff, u8 len)
{
	int i;
	int ret;
	struct i2c_msg msg;
	u8 cmd[I2C_MAX_TRAN];
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di || !di->client || !buff) {
		hwlog_err("di or buff is null\n");
		return -FG_ERR_PARA_NULL;
	}

	cmd[0] = reg;
	memcpy(&cmd[1], buff, len);
	msg.buf = cmd;
	msg.addr = di->client->addr;
	msg.flags = 0;
	msg.len = len + 1; /* reg + len */

	if (addr == FG_ROM_MODE_I2C_ADDR_CFG)
		msg.addr = FG_ROM_MODE_I2C_ADDR;

	mutex_lock(&di->rd_mutex);
	for (i = 0; i < I2C_RETRY_CNT; i++) {
		ret = i2c_transfer(di->client->adapter, &msg, 1);
		if (ret >= 0)
			break;

		usleep_range(5000, 5100); /* sleep 5ms */
	}
	mutex_unlock(&di->rd_mutex);

	if (ret < 0) {
		hwlog_err("write_block failed[%x]\n", reg);
		return -FG_ERR_I2C_W;
	}

	return 0;
}

static int bq27z561_write_word(u8 reg, u16 data)
{
	/* 4 bytes offset 2 contains the data offset 0 is used by i2c_write */
	u8 buff[4] = { 0 };

	put_unaligned_le16(data, &buff[0]);
	/* 2: one word */
	return bq27z561_write_block(INVALID_REG_ADDR, reg, buff, 2);
}

static u8 checksum(u8 *data, u8 len)
{
	u8 i;
	u16 sum = 0;

	for (i = 0; i < len; i++)
		sum += data[i];

	sum &= 0xFF; /* use 8 bit */

	return 0xFF - sum;
}

static void bq27z561_print_buf(const char *msg, u8 *buf, u8 len)
{
	int i;
	int idx = 0;
	u8 strbuf[FG_MAX_BUFFER_LEN] = { 0 };

	if (len > FG_MAX_BUFFER_LEN) {
		hwlog_err("data len is over buffer\n");
		return;
	}
	for (i = 0; i < len; i++)
		idx += sprintf(&strbuf[idx], "%02X ", buf[i]);
	hwlog_info("%s\n", strbuf);
}

static int bq27z561_read_mac_data(u16 cmd, u8 *buf, u8 len)
{
	int i;
	int ret;
	u8 t_len;
	u8 cksum;
	u8 cksum_calc;
	u8 t_buf[FG_MAC_TRAN_BUFF] = { 0 }; /* max buf size is 40 bytes */
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di || (len > FG_MAC_WR_MAX))
		return -EINVAL;

	/* 8: u16 to u8 buffer */
	t_buf[1] = (u8)(cmd >> 8);
	t_buf[0] = (u8)cmd;

	mutex_lock(&di->mac_mutex);
	ret = bq27z561_write_block(INVALID_REG_ADDR,
		di->regs[BQ_FG_REG_ALT_MAC], t_buf, MSG_LEN);
	if (ret < 0)
		goto read_mac_out;

	msleep(100); /* delay 100ms for write succ */

	/* 36: max buf size is 36 bytes */
	ret = bq27z561_read_block(INVALID_REG_ADDR,
		di->regs[BQ_FG_REG_ALT_MAC], t_buf, 36);
	if (ret < 0)
		goto read_mac_out;

	cksum = t_buf[34]; /* 34: index checksum */
	t_len = t_buf[35]; /* 34: index len */
	/* 2: here length includes checksum byte and length byte itself */
	if (t_len - 2 > FG_MAC_TRAN_BUFF)
		goto read_mac_out;
	cksum_calc = checksum(t_buf, t_len - 2);
	if (cksum_calc != cksum) {
		ret = -ENODATA;
		goto read_mac_out;
	}

	/* ignore command code, return data field */
	for (i = 0; i < len; i++)
		buf[i] = t_buf[i + FG_MAC_ADDR_LEN];

read_mac_out:
	mutex_unlock(&di->mac_mutex);
	return ret;
}

static int bq27z561_write_mac_data(u16 cmd, u8 *data, u8 len)
{
	int i;
	int ret;
	u8 cksum;
	u8 t_buf[FG_MAC_TRAN_BUFF] = { 0 };
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di || (len > FG_MAC_WR_MAX))
		return -EINVAL;

	/* 8: u16 to u8 buffer */
	t_buf[1] = (u8)(cmd >> 8);
	t_buf[0] = (u8)cmd;

	for (i = 0; i < len; i++)
		t_buf[i + FG_MAC_ADDR_LEN] = data[i];

	/* write command/addr, data */
	cksum = checksum(t_buf, len + FG_MAC_ADDR_LEN);

	mutex_lock(&di->mac_mutex);
	ret = bq27z561_write_block(INVALID_REG_ADDR,
		di->regs[BQ_FG_REG_ALT_MAC], t_buf,
		len + FG_MAC_ADDR_LEN);
	if (ret < 0)
		goto write_mac_out;
	bq27z561_print_buf("wr mac data", t_buf, len + FG_MAC_ADDR_LEN);
	t_buf[0] = cksum;
	t_buf[1] = len + 4; /* 4: buf length, cmd, CRC and len byte itself */
	/* 2: write checksum and length */
	ret = bq27z561_write_block(INVALID_REG_ADDR,
		di->regs[BQ_FG_REG_MAC_CHKSUM], t_buf, 2);

write_mac_out:
	msleep(FG_WRITE_MAC_DELAY_TIME);
	mutex_unlock(&di->mac_mutex);
	return ret;
}

static void bq27z561_read_fw_version(void)
{
	int ret;
	u8 buf[FG_MAC_WR_MAX] = { 0 };

	ret = bq27z561_read_mac_data(FG_MAC_CMD_FW_VER, buf, MAC_DATA_LEN);
	if (ret < 0) {
		hwlog_err("failed to read firmware version:%d\n", ret);
		return;
	}

	/* buf[2~5]:firmware version */
	hwlog_info("fw ver:%04X, build:%04X\n",
		buf[2] << 8 | buf[3], buf[4] << 8 | buf[5]);
	/* buf[7~8]:ztrack version */
	hwlog_info("ztrack ver:%04X\n", buf[7] << 8 | buf[8]);
}

static int bq27z561_get_chem_id(void)
{
	int ret;
	u16 chem_id;
	u8 buf[FG_MAC_WR_MAX] = { 0 };

	ret = bq27z561_read_mac_data(FG_MAC_CMD_CHEM_ID, buf, MSG_LEN);
	if (ret < 0) {
		hwlog_err("failed to read chem id:%d\n", ret);
		return -FG_ERR_I2C_R;
	}
	/* chem_id: u16 */
	chem_id = (buf[0] << 8) | buf[1];

	return chem_id;
}

static int  bq27z561_read_op_status(void)
{
	int ret;
	int op_status;
	u8 buf[FG_MAC_WR_MAX] = { 0 };

	ret = bq27z561_read_mac_data(FG_MAC_CMD_OP_STATUS, buf, MSG_LEN);
	if (ret < 0) {
		hwlog_err("failed to read op status:%d\n", ret);
		return -FG_ERR_I2C_R;
	}
	/* op_status: u16 */
	op_status = (buf[0] << 8) | buf[1];

	return op_status;
}

static int  bq27z561_read_charging_status(void)
{
	int ret;
	int charging_status;
	u8 buf[FG_MAC_WR_MAX] = { 0 };

	ret = bq27z561_read_mac_data(FG_MAC_CMD_CHARGING_STATUS, buf, MSG_LEN);
	if (ret < 0) {
		hwlog_err("failed to read charging status:%d\n", ret);
		return -FG_ERR_I2C_R;
	}
	/* charging_status: u16 */
	charging_status = (buf[0] << 8) | buf[1];

	return charging_status;
}

static int bq27z561_read_gauging_status(void)
{
	int ret;
	int gauging_status;
	u8 buf[FG_MAC_WR_MAX] = { 0 };

	ret = bq27z561_read_mac_data(FG_MAC_CMD_GAUGING_STATUS, buf, DWORD_LEN);
	if (ret < 0) {
		hwlog_err("failed to read gauging status:%d\n", ret);
		return -FG_ERR_I2C_R;
	}
	/* u32 buf[0~3]:gauging_status */
	gauging_status = (buf[0] << 24) | (buf[1] << 16) |
		(buf[2] << 8) | buf[3];

	return gauging_status;
}

static int bq27z561_read_manu_status(void)
{
	int ret;
	int manu_status;
	u8 buf[FG_MAC_WR_MAX] = { 0 };

	ret = bq27z561_read_mac_data(FG_MAC_CMD_MANU_STATUS, buf, MSG_LEN);
	if (ret < 0) {
		hwlog_err("failed to read manu status:%d\n", ret);
		return -FG_ERR_I2C_R;
	}
	/* manu_status: u16 */
	manu_status = (buf[1] << 8) | buf[0];

	return manu_status;
}

static int bq27z561_read_para_ver(void)
{
	int ret;
	int version;
	u8 buf[FG_MAC_WR_MAX] = { 0 };

	ret = bq27z561_read_mac_data(FG_MAC_CMD_MANU_INFO, buf, BYTE_LEN);
	if (ret < 0) {
		hwlog_err("failed to read para ver:%d\n", ret);
		return -FG_ERR_I2C_R;
	}
	version = buf[0];

	return version;
}

static int bq27z561_is_ready(void *dev_data)
{
	if (g_bq27z561_dev)
		return 1;

	return 0;
}

static int bq27z561_read_status(struct bq27z561_device_info *di)
{
	int ret;
	u16 flags = 0;

	ret = bq27z561_read_word(di->regs[BQ_FG_REG_BATT_STATUS], &flags);
	if (ret < 0)
		return ret;

	di->batt_fc = !!(flags & FG_FLAGS_FC);
	di->batt_fd = !!(flags & FG_FLAGS_FD);
	di->batt_rca = !!(flags & FG_FLAGS_RCA);
	di->batt_dsg = !!(flags & FG_FLAGS_DSG);

	return 0;
}

static int bq27z561_read_rsoc(void)
{
	int ret;
	u16 soc = 0;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq27z561_read_word(di->regs[BQ_FG_REG_SOC], &soc);
	if (ret < 0) {
		hwlog_err("could not read rsoc, ret = %d\n", ret);
		return 0;
	}

	return soc;
}

static int bq27z561_read_last_capacity(void)
{
	int ret;
	u8 t_buf[DWORD_LEN] = { 0 };
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq27z561_read_mac_data(FG_MAC_CMD_SOC_CFG, t_buf, DWORD_LEN);
	if (ret < 0) {
		hwlog_err("failed to read last capacity:%d\n", ret);
		return 0;
	}

	return t_buf[0];
}

static int bq27z561_read_control_status(void)
{
	int ret;
	u16 control_status = 0;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq27z561_read_word(di->regs[BQ_FG_REG_CTRL], &control_status);
	if (ret < 0) {
		hwlog_err("could not read control status, ret = %d\n", ret);
		return 0;
	}

	return control_status;
}

static int bq27z561_read_temperature(void)
{
	int ret;
	u16 temp = 0;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq27z561_read_word(di->regs[BQ_FG_REG_TEMP], &temp);
	if (ret < 0) {
		hwlog_err("could not read temperature, ret = %d\n", ret);
		return 0;
	}

	return temp - 2730; /* 2730: base temp */
}

static int bq27z561_is_battery_exist(void *dev_data)
{
	int temp;
	struct bq27z561_device_info *di = g_bq27z561_dev;

#ifdef CONFIG_HLTHERM_RUNTEST
	return 0;
#endif /* CONFIG_HLTHERM_RUNTEST */

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	temp = di->batt_temp;

	if ((temp <= ABNORMAL_BATT_TEMPERATURE_LOW) ||
		(temp >= ABNORMAL_BATT_TEMPERATURE_HIGH))
		return 0;

	return 1; /* battery is existing */
}

static int bq27z561_read_volt(void)
{
	int ret;
	u16 volt = 0;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq27z561_read_word(di->regs[BQ_FG_REG_VOLT], &volt);
	if (ret < 0) {
		hwlog_err("could not read voltage, ret = %d\n", ret);
		return 0;
	}

	return volt;
}

static int bq27z561_read_current(void)
{
	int ret;
	int curr;
	u16 curr_tmp = 0;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq27z561_read_word(di->regs[BQ_FG_REG_I], &curr_tmp);
	if (ret < 0) {
		hwlog_err("could not read current, ret = %d\n", ret);
		return 0;
	}
	curr = (int)((s16)curr_tmp);

	return curr;
}

static int bq27z561_read_current_avg(void)
{
	int ret;
	int curr;
	u16 avg_curr = 0;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	ret = bq27z561_read_word(di->regs[BQ_FG_REG_AI], &avg_curr);
	if (ret < 0) {
		hwlog_err("could not read current, ret = %d\n", ret);
		return 0;
	}
	curr = (int)((s16)avg_curr);

	return curr;
}

static int bq27z561_read_fcc(void)
{
	int ret;
	u16 fcc = 0;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	if (di->regs[BQ_FG_REG_FCC] == INVALID_REG_ADDR) {
		hwlog_err("FCC command not supported\n");
		return 0;
	}
	ret = bq27z561_read_word(di->regs[BQ_FG_REG_FCC], &fcc);
	if (ret < 0)
		hwlog_err("could not read FCC, ret = %d\n", ret);

	return fcc;
}

static int bq27z561_read_dc(void)
{
	int ret;
	u16 dc = 0;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	if (di->regs[BQ_FG_REG_DC] == INVALID_REG_ADDR) {
		hwlog_err("DesignCapacity command not supported\n");
		return 0;
	}
	ret = bq27z561_read_word(di->regs[BQ_FG_REG_DC], &dc);
	if (ret < 0) {
		hwlog_err("could not read DC, ret = %d\n", ret);
		return 0;
	}

	return dc;
}

static int bq27z561_read_rm(void)
{
	int ret;
	u16 rm = 0;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	if (di->regs[BQ_FG_REG_RM] == INVALID_REG_ADDR) {
		hwlog_err("RemainingCapacity command not supported\n");
		return 0;
	}
	ret = bq27z561_read_word(di->regs[BQ_FG_REG_RM], &rm);
	if (ret < 0) {
		hwlog_err("could not read DC, ret = %d\n", ret);
		return 0;
	}

	return rm;
}

static int bq27z561_read_cyclecount(void)
{
	int ret;
	u16 cc = 0;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	if (di->regs[BQ_FG_REG_CC] == INVALID_REG_ADDR) {
		hwlog_err("Cycle Count not supported\n");
		return -FG_ERR_MISMATCH;
	}
	ret = bq27z561_read_word(di->regs[BQ_FG_REG_CC], &cc);
	if (ret < 0) {
		hwlog_err("could not read Cycle Count, ret = %d\n", ret);
		return ret;
	}

	return cc;
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
static int bq27z561_read_tte(void)
{
	int ret;
	u16 tte = 0;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	if (di->regs[BQ_FG_REG_TTE] == INVALID_REG_ADDR) {
		hwlog_err("time to empty not supported\n");
		return -FG_ERR_MISMATCH;
	}
	ret = bq27z561_read_word(di->regs[BQ_FG_REG_TTE], &tte);
	if (ret < 0) {
		hwlog_err("could not read Time To Empty, ret = %d\n", ret);
		return 0;
	}
	if (ret == 0xFFFF) /* illegal image data */
		return -ENODATA;

	return tte;
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static int bq27z561_get_batt_status(struct bq27z561_device_info *di)
{
	bq27z561_read_status(di);
	if (di->batt_fc)
		return POWER_SUPPLY_STATUS_FULL;
	else if (di->batt_dsg)
		return POWER_SUPPLY_STATUS_DISCHARGING;
	else if (di->batt_curr > 0)
		return POWER_SUPPLY_STATUS_CHARGING;

	return POWER_SUPPLY_STATUS_NOT_CHARGING;
}

static int bq27z561_read_qmax(void)
{
	int ret;
	int qmax;
	u8 t_buf[MSG_LEN] = { 0 };

	ret = bq27z561_read_mac_data(FG_MAC_CMD_QMAX_CELL, t_buf, MSG_LEN);
	if (ret < 0)
		return 0;

	/* qmax: u16 */
	qmax = (t_buf[1] << 8) | t_buf[0];

	return qmax;
}

#ifdef POWER_MODULE_DEBUG_FUNCTION
static int bq27z561_get_batt_capacity_level(struct bq27z561_device_info *di)
{
	if (di->batt_fc)
		return POWER_SUPPLY_CAPACITY_LEVEL_FULL;
	else if (di->batt_rca)
		return POWER_SUPPLY_CAPACITY_LEVEL_LOW;
	else if (di->batt_fd)
		return POWER_SUPPLY_CAPACITY_LEVEL_CRITICAL;

	return POWER_SUPPLY_CAPACITY_LEVEL_NORMAL;
}

static void bq27z561_show_ra_table(void)
{
	int i;
	int ret;
	u8 t_buf[I2C_BLK_SIZE] = { 0 };

	ret = bq27z561_read_mac_data(FG_MAC_CMD_RA_TABLE, t_buf, I2C_BLK_SIZE);
	if (ret < 0)
		return;

	/* ra_table: 16*u16 */
	hwlog_info("ra_table:\n");
	for (i = 0; i < I2C_BLK_SIZE_HALF; i++)
		hwlog_info("0x%x\n", t_buf[i * 2] << 8 | t_buf[i * 2 + 1]);
}

static void bq27z561_show_cell_gain(void)
{
	int ret;
	int cell_gain;
	u8 buf[MSG_LEN] = { 0 };

	ret = bq27z561_read_mac_data(FG_MAC_CMD_CELL_GAIN, buf, MSG_LEN);
	if (ret < 0)
		return;

	/* cell_gain: u16 */
	cell_gain = (buf[0] << 8) | buf[1];
	hwlog_info("cell_gain:0x%x\n", cell_gain);
}

static void bq27z561_show_cc_gain(void)
{
	int ret;
	int cc_gain;
	u8 buf[DWORD_LEN] = { 0 };

	ret = bq27z561_read_mac_data(FG_MAC_CMD_CC_GAIN,
		buf, DWORD_LEN);
	if (ret < 0)
		return;

	/* u32:buf[0~3]:cc_gain */
	cc_gain = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	hwlog_info("cc_gain:0x%x\n", cc_gain);
}

static void bq27z561_show_cap_gain(void)
{
	int ret;
	int cap_gain;
	u8 buf[DWORD_LEN] = { 0 };

	ret = bq27z561_read_mac_data(FG_MAC_CMD_CAP_GAIN, buf, DWORD_LEN);
	if (ret < 0)
		return;
	/* u32:buf[0~3]:cap_gain */
	cap_gain = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	hwlog_info("cap_gain:0x%x\n", cap_gain);
}

static void bq27z561_show_update_status(void)
{
	int ret;
	u8 t_buf[MSG_LEN] = { 0 };

	ret = bq27z561_read_mac_data(FG_MAC_CMD_UPDATE_STATUS,
		t_buf, BYTE_LEN);
	if (ret < 0)
		return;

	/* update_status: u8 */
	hwlog_info("update status:0x%x\n", t_buf[0]);
}
#endif /* POWER_MODULE_DEBUG_FUNCTION */

static int bq27z561_get_battery_capacity(void *dev_data)
{
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -FG_ERR_PARA_NULL;
	}

	return di->batt_soc;
}

static int bq27z561_get_battery_voltage(void *dev_data)
{
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -FG_ERR_PARA_NULL;
	}
	if (atomic_read(&di->is_update))
		return di->batt_volt;

	return bq27z561_read_volt();
}

static int bq27z561_get_battery_current(void *dev_data)
{
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -FG_ERR_PARA_NULL;
	}
	if (atomic_read(&di->is_update))
		return di->batt_curr;

	return bq27z561_read_current();
}

static int bq27z561_get_battery_avg_current(void *dev_data)
{
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -FG_ERR_PARA_NULL;
	}
	if (atomic_read(&di->is_update))
		return di->batt_curr_avg;

	return bq27z561_read_current_avg();
}

static int bq27z561_set_ntc_compensation_temp(struct bq27z561_device_info *pdata,
	int temp_val, int cur_temp)
{
	int temp_with_compensation = temp_val;
	struct common_comp_data comp_data;

	if (!pdata)
		return temp_with_compensation;

	comp_data.refer = abs(cur_temp);
	comp_data.para_size = NTC_PARA_LEVEL;
	comp_data.para = pdata->ntc_temp_compensation_para;
	if (pdata->ntc_compensation_is == 1)
		temp_with_compensation = power_get_compensation_value(temp_val,
			&comp_data);

	hwlog_debug("temp_with_compensation=%d temp_no_compensation=%d ichg=%d\n",
		temp_with_compensation, temp_val, cur_temp);
	return temp_with_compensation;
}

static int bq27z561_get_battery_temperature(void *dev_data)
{
	struct bq27z561_device_info *di = g_bq27z561_dev;
	int temp;
	int bat_curr;

	if (!di) {
		hwlog_err("di is null\n");
		return -FG_ERR_PARA_NULL;
	}
	bat_curr = bq27z561_get_battery_current(NULL);
	if (atomic_read(&di->is_update))
		temp = di->batt_temp;
	else
		temp = bq27z561_read_temperature();

	return bq27z561_set_ntc_compensation_temp(di, temp, bat_curr);
}

static int bq27z561_get_battery_fcc(void *dev_data)
{
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -FG_ERR_PARA_NULL;
	}

	return di->batt_fcc;
}

static int bq27z561_get_battery_rm(void)
{
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -FG_ERR_PARA_NULL;
	}
	if (atomic_read(&di->is_update))
		return di->batt_rm;

	return bq27z561_read_rm();
}

static int bq27z561_get_battery_cycle(void *dev_data)
{
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -FG_ERR_PARA_NULL;
	}

	return di->batt_cyclecnt;
}

static int bq27z561_get_battery_qmax(void)
{
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return -FG_ERR_PARA_NULL;
	}

	return di->batt_qmax;
}

static void bq27z561_get_battery_info(struct bq27z561_device_info *di)
{
	if (!di)
		return;

	di->batt_soc = bq27z561_read_rsoc();
	di->batt_fcc = bq27z561_read_fcc();
	di->batt_volt = bq27z561_read_volt();
	di->batt_temp = bq27z561_read_temperature();
	di->batt_curr = bq27z561_read_current();
	di->batt_curr_avg = bq27z561_read_current_avg();
	di->batt_cyclecnt = bq27z561_read_cyclecount();
	di->batt_qmax = bq27z561_read_qmax();
	di->batt_rm = bq27z561_read_rm();
}

static int bq27z561_set_battery_low_voltage(int val, void *dev_data)
{
	int ret;

	if ((val <= 0) || (val > BQ27Z561_VOLT_OVER_VAL))
		return -FG_ERR_PARA_WRONG;

	ret = bq27z561_write_word(BQ27Z561_VOLT_LOW_SET_REG, val);
	ret += bq27z561_write_word(BQ27Z561_VOLT_LOW_CLR_REG,
		val + BQ27Z561_VOLT_LOW_CLR_OFFSET);

	return ret;
}

static int bq27z561_set_last_capacity(int capacity, void *dev_data)
{
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di || (capacity > FG_FULL_CAPACITY) || (capacity < 0) ||
		atomic_read(&di->is_update))
		return -FG_ERR_PARA_WRONG;

	return bq27z561_write_mac_data(FG_MAC_CMD_SOC_CFG,
		(u8 *)&capacity, sizeof(capacity));
}

static int bq27z561_get_last_capacity(void *dev_data)
{
	int ret;
	int last_cap;
	int cap;
	u8 t_buf[DWORD_LEN] = { 0 };
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di)
		return 0;

	if (atomic_read(&di->is_update)) {
		cap = di->batt_soc;
		last_cap = di->last_batt_soc;
	} else {
		ret = bq27z561_read_mac_data(FG_MAC_CMD_SOC_CFG,
			t_buf, MSG_LEN);
		if (ret < 0) {
			hwlog_err("failed to read last capacity:%d\n", ret);
			return 0;
		}
		last_cap = t_buf[0];
		cap = bq27z561_read_rsoc();
	}

	hwlog_info("read cap=%d, last_cap=%d\n", cap, last_cap);
	if (((last_cap <= 0) || (cap <= 0)) ||
		(abs(last_cap - cap) >= FG_CAPACITY_TH))
		return cap;

	return last_cap;
}

static int bq27z561_dump_log_data(char *buffer, int size, void *dev_data)
{
	struct bq27z561_device_info *di = dev_data;
	struct bq27z561_display_data g_dis_data;

	if (!buffer || !di)
		return -FG_ERR_PARA_NULL;

	g_dis_data.vbat = bq27z561_get_battery_voltage(NULL);
	g_dis_data.ibat = bq27z561_get_battery_current(NULL);
	g_dis_data.avg_ibat = bq27z561_get_battery_avg_current(NULL);
	g_dis_data.rm = bq27z561_get_battery_rm();
	g_dis_data.temp = bq27z561_get_battery_temperature(NULL);
	g_dis_data.soc = bq27z561_get_battery_capacity(NULL);
	g_dis_data.fcc = bq27z561_get_battery_fcc(NULL);
	g_dis_data.qmax = bq27z561_get_battery_qmax();

	snprintf(buffer, size, "%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d   ",
		g_dis_data.temp, g_dis_data.vbat, g_dis_data.ibat,
		g_dis_data.avg_ibat, g_dis_data.rm, g_dis_data.soc,
		g_dis_data.fcc, g_dis_data.qmax);

	return 0;
}

static int bq27z561_get_log_head(char *buffer, int size, void *dev_data)
{
	struct bq27z561_device_info *di = dev_data;

	if (!buffer || !di)
		return -FG_ERR_PARA_NULL;

	snprintf(buffer, size,
		"    Temp   Vbat   Ibat   AIbat   Rm   Soc   Fcc   Qmax");

	return 0;
}

static struct power_log_ops bq27z561_fg_ops = {
	.dev_name = "bq27z561",
	.dump_log_head = bq27z561_get_log_head,
	.dump_log_content = bq27z561_dump_log_data,
};

static struct coul_interface_ops bq27z561_ops = {
	.type_name = "main",
	.is_coul_ready = bq27z561_is_ready,
	.is_battery_exist = bq27z561_is_battery_exist,
	.get_battery_capacity = bq27z561_get_battery_capacity,
	.get_battery_voltage = bq27z561_get_battery_voltage,
	.get_battery_current = bq27z561_get_battery_current,
	.get_battery_avg_current = bq27z561_get_battery_avg_current,
	.get_battery_temperature = bq27z561_get_battery_temperature,
	.get_battery_fcc = bq27z561_get_battery_fcc,
	.get_battery_cycle = bq27z561_get_battery_cycle,
	.set_battery_low_voltage = bq27z561_set_battery_low_voltage,
	.set_battery_last_capacity = bq27z561_set_last_capacity,
	.get_battery_last_capacity = bq27z561_get_last_capacity,
};

static void bq27z561_dump_registers(struct bq27z561_device_info *di)
{
	int i;
	int ret;
	u16 val = 0;

	for (i = 0; i < ARRAY_SIZE(bq27z561_dump_regs); i++) {
		usleep_range(5000, 5100); /* sleep 5ms */
		ret = bq27z561_read_word(bq27z561_dump_regs[i], &val);
		if (!ret)
			hwlog_err("Reg[%02X] = 0x%04X\n", bq27z561_dump_regs[i], val);
	}
}

static int bq27z561_flash_mode(void)
{
	u8 cmd = 0x11; /* flash mode cmd */

	/* change to flash mode, 0x08: reg address */
	if (bq27z561_write_block(FG_ROM_MODE_I2C_ADDR_CFG, 0x08, &cmd, 1)) {
		hwlog_err("change to flash mode error\n");
		return -FG_ERR_I2C_W;
	}
	msleep(FG_SW_MODE_DELAY_TIME);

	return 0;
}

static int bq27z561_update_bqfs_write_block(u16 addr, u8 reg, u8 *buf, u8 len)
{
	int ret;
	u8 wr_len = 0;

	if (!buf) {
		hwlog_err("buf null\n");
		return -FG_ERR_PARA_NULL;
	}

	while (len > I2C_BLK_SIZE) {
		ret = bq27z561_write_block(addr, reg + wr_len,
			buf + wr_len, I2C_BLK_SIZE);
		if (ret)
			return -FG_ERR_I2C_W;
		wr_len += I2C_BLK_SIZE;
		len -= I2C_BLK_SIZE;
	}

	if (len > 0) {
		ret = bq27z561_write_block(addr, reg + wr_len,
			buf + wr_len, len);
		if (ret)
			return -FG_ERR_I2C_W;
	}

	return 0;
}

static int bq27z561_update_bqfs_execute_cmd(const struct bqfs_cmd_info *cmd)
{
	int ret;
	u8 tmp_buf[I2C_BLK_SIZE] = { 0 };

	if (!cmd) {
		hwlog_err("cmd null\n");
		return -FG_ERR_PARA_NULL;
	}

	switch (cmd->cmd_type) {
	case CMD_R:
		ret = bq27z561_read_block(cmd->addr, cmd->reg,
			(u8 *)&cmd->data.bytes, cmd->data_len);
		if (ret)
			return -FG_ERR_I2C_R;
		break;
	case CMD_W:
		ret = bq27z561_update_bqfs_write_block(cmd->addr, cmd->reg,
			(u8 *)&cmd->data.bytes, cmd->data_len);
		if (ret)
			return -FG_ERR_I2C_W;
		break;
	case CMD_C:
		ret = bq27z561_read_block(cmd->addr, cmd->reg,
			tmp_buf, cmd->data_len);
		if (ret)
			return -FG_ERR_I2C_R;

		msleep(BQ27Z561_DELAY_TIME);
		if (memcmp(tmp_buf, cmd->data.bytes, cmd->data_len)) {
			hwlog_err("mismatch data_len = 0x%x\n", cmd->data_len);
			return -FG_ERR_MISMATCH;
		}
		break;
	case CMD_X:
		hwlog_info("delay=0x%x\n", cmd->data.delay);
		msleep(cmd->data.delay);
		break;
	default:
		hwlog_info("unsupport cmd at line %d\n", cmd->line_num);
		return -FG_ERR_I2C_R;
	}

	return 0;
}

static int bq27z561_update_bqfs_image(void)
{
	int i;
	int len;
	int ret;
	u32 rec_cnt = 0;
	const u8 *p = NULL;
	struct bqfs_cmd_info cmd = { 0 };
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di || !di->bqfs_image_data) {
		hwlog_err("di null\n");
		return -FG_ERR_PARA_NULL;
	}
	hwlog_info("update bqfs begin count=%d\n", di->count);
	p = di->bqfs_image_data;
	while (p < di->bqfs_image_data + di->count) {
		cmd.cmd_type = *p++;
		if (cmd.cmd_type == CMD_X) {
			len = *p++;
			if (len != BQ27Z561_WORD_LEN) {
				hwlog_err("update bqfs fail, len = %d\n", len);
				goto update_bqfs_err;
			}

			cmd.data.delay = *(p + 1) | *p << 8; /* u8 to u16 */
			p += BQ27Z561_WORD_LEN;
		} else {
			cmd.addr = *p++;
			cmd.reg  = *p++;
			cmd.data_len = *p++;
			for (i = 0; i < cmd.data_len; i++)
				cmd.data.bytes[i] = *p++;
		}

		rec_cnt++;

		if (bq27z561_update_bqfs_execute_cmd(&cmd)) {
			hwlog_err("execute cmd fail, rec_cnt = %d\n", rec_cnt);
			goto update_bqfs_err;
		}
		usleep_range(1000, 1050); /* 1ms */
	}

	return 0;
update_bqfs_err:
	di->batt_version = FG_PARA_INVALID_VER;
	ret = bq27z561_flash_mode();
	if (ret)
		hwlog_err("switch to flash mode fail\n");
	return -1; /* update bqfs fail */
}

static int bq27z561_decimal_to_binary(char *res, int res_size, u64 num,
	u64 prec, int pos)
{
	int count = res_size - pos;
	int first = -1; /* initial value */
	bool find = false;

	/* whether the input parameter is correct */
	if ((num == 0) || (pos >= (res_size - 1)))
		return pos;

	if (prec == 0)
		return -EINVAL;

	while (count > 0) {
		num <<= 1; /* move one bit to the left */
		if (num / prec) {
			res[pos++] = 1; /* binary bit */
			num = num % prec;
			if (!find) {
				first = pos - 1; /* get next cur_pos */
				find = true;
			}
		} else {
			res[pos++] = 0; /* binary bit */
		}
		count--;
	}

	return first;
}

static int bq27z561_integer_to_binary(char *res, unsigned int res_size,
	u64 num, unsigned int pos)
{
	int tmp_pos = 0;
	char tmp[FLOAT_BIT_NUMS] = { 0 };

	if (num == 0) {
		res[pos] = 0; /* set error value */
		pos++;
		return pos;
	}

	while (num > 0) {
		tmp[tmp_pos] = num & 1; /* get the last binary bit */
		num >>= 1; /* move one bit to the right to get next binary bit */
		tmp_pos++;
		if (tmp_pos > FLOAT_BIT_NUMS)
			return -ENOMEM;
	}

	while ((tmp_pos > 0) && (pos < res_size))
		res[pos++] = tmp[--tmp_pos];

	return pos;
}

static void bq27z561_get_fraction(struct spf *spf, char *res, int count, int pos)
{
	u32 tmp = 0;
	int i = 0;
	bool first = true;

	while (i < count) {
		if (res[pos] == FLOAT_POINT_BIT) {
			pos++;
			continue;
		}

		if (!first)
			tmp <<= 1; /* move one bit to the left */
		else
			first = false;

		tmp |= res[pos];
		i++;
		pos++;
	}
	spf->fraction = tmp & 0x7fffff; /* get low 23 bits */
}

static void bq27z561_get_single_float_body(struct spf *spf, struct fep *fe, char *res)
{
	int t = fe->first_one_pos - fe->dec_point_pos;

	/* set exponential byte */
	if (t > 0) {
		spf->exponent = FLOAT_EXPONENTIAL_DEFAULT & ~t;
	} else {
		t = -t;
		if (t == 1)
			spf->exponent = FLOAT_EXPONENTIAL_DEFAULT;
		else
			spf->exponent = FLOAT_EXPONENTIAL_SIGN_BIT |
				(FLOAT_EXPONENTIAL_DEFAULT & (t - FLOAT_EXPONENTIAL_REDUCE_VALUE));
	}

	bq27z561_get_fraction(spf, res, FLOAT_FRACTION_SIZE, fe->first_one_pos + 1);
}

static int bq27z561_to_binary(char *res, unsigned int res_size, u64 num,
	u64 prec, struct fep *fe)
{
	int cur_pos = 0;

	if (prec == 0)
		return -EINVAL;

	cur_pos = bq27z561_integer_to_binary(res, res_size, num / prec, cur_pos);
	if (cur_pos < 0)
		return -EINVAL;

	fe->dec_point_pos = cur_pos;
	res[cur_pos++] = FLOAT_POINT_BIT;

	bq27z561_decimal_to_binary(res, res_size, num % prec, prec, cur_pos);
	return 0;
}

static u32 bq27z561_float_convert(s64 num, u64 prec)
{
	int i = 0;
	char bits[FLOAT_BIT_NUMS] = { 0 };
	struct spf mf;
	struct fep fe;
	u32 *res = NULL;

	if (num == 0)
		return 0;

	/* get sign bit */
	mf.sign = (num >= 0) ? 0 : 1;

	/* convert number to positive */
	if (num < 0)
		num = -num;

	/* convert to binary */
	if (bq27z561_to_binary(bits, FLOAT_BIT_NUMS, (u64)num, prec, &fe))
		return 0;

	/* get the position of the first binary one */
	while (i < FLOAT_BIT_NUMS) {
		if (bits[i] == 1)
			break;
		i++;
	}

	if (i < FLOAT_BIT_NUMS)
		fe.first_one_pos = i;
	else
		return 0;

	/* get exponent and fraction part */
	bq27z561_get_single_float_body(&mf, &fe, bits);

	/* convert to 4 bytes Integer */
	res = (u32 *)&mf;

	return *res;
}

static int bq27z561_get_calibration_adc(int type, int *val)
{
	int ret;
	s64 tmp_vol;
	s64 tmp_cur;
	u8 t_buf[FG_MAC_TRAN_BUFF] = { 0 };
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di || atomic_read(&di->is_update)) {
		*val = 0;
		return -FG_ERR_PARA_NULL;
	}

	/* 24: max buf size is 24 bytes */
	ret = bq27z561_read_block(INVALID_REG_ADDR,
		di->regs[BQ_FG_REG_ALT_MAC], t_buf, 24);
	if (ret < 0) {
		hwlog_err("get_calibration_vol %d\n", ret);
		*val = 0;
		return -FG_ERR_I2C_R;
	}
	if (type == BQ27Z561_CALI_VOL) {
		/* buf[6~7] is cell voltage */
		tmp_vol = (t_buf[7] << 8 | t_buf[6]) *
			BQ27Z561_CELL_GAIN_DEFAULT / BQ27Z561_CALI_UNIT;
		hwlog_info("cell voltage:0x%x\n", t_buf[7] << 8 | t_buf[6]);
		tmp_vol = tmp_vol * di->vol_gain / BQ27Z561_CALI_PRECIE;
		*val = (int)(tmp_vol * 1000); /* mv to uv */
	} else {
		/* buf[4~5] is current */
		tmp_cur = (t_buf[5] << 8 | t_buf[4]);
		hwlog_info("current:0x%x\n", t_buf[5] << 8 | t_buf[4]);
		if (tmp_cur < BQ27Z561_CALI_CUR_TH)
			tmp_cur = tmp_cur * BQ27Z561_CALI_CUR_MA /
				BQ27Z561_CALI_CUR_BASE;
		else
			tmp_cur = -(BQ27Z561_CALI_CUR_MAX - tmp_cur + 1) *
				BQ27Z561_CALI_CUR_MA / BQ27Z561_CALI_CUR_BASE;
		*val = (int)(tmp_cur * di->cur_gain / BQ27Z561_CALI_PRECIE);
	}

	return 0;
}

static int bq27z561_get_calibration_curr(int *val, void *data)
{
	return bq27z561_get_calibration_adc(BQ27Z561_CALI_CUR, val);
}

static int bq27z561_get_calibration_vol(int *val, void *data)
{
	return bq27z561_get_calibration_adc(BQ27Z561_CALI_VOL, val);
}

static int bq27z561_enter_calibration_mode(void)
{
	int ret;
	int manu_status;
	u8 io_cfg = 0;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di)
		return -FG_ERR_PARA_NULL;

	cancel_delayed_work_sync(&di->batt_para_monitor_work);
	manu_status = bq27z561_read_manu_status();
	hwlog_info("manu_status = 0x%x\n", manu_status);
	ret = bq27z561_write_mac_data(FG_MAC_CMD_IO_CONFIG,
		&io_cfg, sizeof(io_cfg));
	if (!(manu_status & BQ27Z561_CALI_EN_DISABLE_MASK))
		ret += bq27z561_write_mac_data(FG_MAC_CMD_CAL_EN, NULL, 0);
	ret += bq27z561_write_mac_data(FG_MAC_CMD_RAW_ADC, NULL, 0);
	if (ret) {
		hwlog_err("enter calibration mode fail\n");
		return ret;
	}
	di->calib_enable = true;

	return 0;
}

static int bq27z561_exit_calibration_mode(void)
{
	int ret;
	u8 io_cfg = FG_IO_CONFIG_VAL;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di)
		return -FG_ERR_PARA_NULL;

	ret = bq27z561_write_mac_data(FG_MAC_CMD_CAL_EN, NULL, 0);
	ret += bq27z561_write_mac_data(FG_MAC_CMD_IO_CONFIG,
		&io_cfg, sizeof(io_cfg));
	if (ret) {
		hwlog_err("exit calibration mode fail\n");
		return ret;
	}
	di->calib_enable = false;
	schedule_delayed_work(&di->batt_para_monitor_work,
		msecs_to_jiffies(0));

	return 0;
}

static int bq27z561_enable_cali_mode(int enable, void *data)
{
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di || atomic_read(&di->is_update))
		return -FG_ERR_PARA_NULL;

	if (enable)
		return bq27z561_enter_calibration_mode();
	else
		return bq27z561_exit_calibration_mode();
}

static int bq27z561_set_cc_gain(unsigned int val)
{
	u32 w_val;

	if ((val < BQ27Z561_CALI_MIN_A) || (val > BQ27Z561_CALI_MAX_A))
		return -FG_ERR_PARA_WRONG;

	/* cc gain default value 368 / 100 */
	w_val = bq27z561_float_convert((s64)val * 368, BQ27Z561_CALI_PRECIE * 100);
	/* 4: CC GAIN cmd len */
	if (bq27z561_write_mac_data(FG_MAC_CMD_CC_GAIN, (u8 *)&w_val, 4) < 0) {
		hwlog_err("write cc gain fail\n");
		return -FG_ERR_I2C_W;
	}

	return 0;
}

static int bq27z561_set_capacity_gain(unsigned int val)
{
	u32 w_val;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di)
		return -FG_ERR_PARA_NULL;

	if ((val < BQ27Z561_CALI_MIN_A) || (val > BQ27Z561_CALI_MAX_A))
		return -FG_ERR_PARA_WRONG;

	w_val = bq27z561_float_convert((s64)val * BQ27Z561_CAP_GAIN_DEFAULT, BQ27Z561_CALI_PRECIE);
	/* 4: CAP GAIN cmd len */
	if (bq27z561_write_mac_data(FG_MAC_CMD_CAP_GAIN, (u8 *)&w_val, 4) < 0) {
		hwlog_err("write capacity gain fail\n");
		return -FG_ERR_I2C_W;
	}
	if (di->calib_enable) {
		if (bq27z561_write_mac_data(FG_MAC_CMD_RAW_ADC, NULL, 0)) {
			hwlog_err("enter calibration mode fail\n");
			return -FG_ERR_I2C_W;
		}
	}

	return 0;
}

static int bq27z561_set_current_gain(unsigned int val, void *data)
{
	int ret;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di)
		return -FG_ERR_PARA_NULL;

	ret = bq27z561_set_cc_gain(val);
	ret += bq27z561_set_capacity_gain(val);
	if (ret)
		return ret;
	di->cur_gain = val;
	hwlog_info("cur_gain %d\n", di->cur_gain);

	return 0;
}

static int bq27z561_set_voltage_gain(unsigned int val, void *data)
{
	u16 tmp;
	u16 *w_val = NULL;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di)
		return -FG_ERR_PARA_NULL;

	if ((val < BQ27Z561_TBATICAL_MIN_A) || (val > BQ27Z561_TBATICAL_MAX_A))
		return -FG_ERR_PARA_WRONG;

	tmp = ((s64)val * BQ27Z561_CELL_GAIN_DEFAULT) / BQ27Z561_CALI_PRECIE;
	w_val = &tmp;
	/* cell gain: 2 bytes */
	if (bq27z561_write_mac_data(FG_MAC_CMD_CELL_GAIN, (u8 *)w_val, 2) < 0) {
		hwlog_err("write cell gain fail\n");
		return -FG_ERR_I2C_W;
	}
	if (di->calib_enable) {
		if (bq27z561_write_mac_data(FG_MAC_CMD_RAW_ADC, NULL, 0)) {
			hwlog_err("enter calibration mode fail\n");
			return -FG_ERR_I2C_W;
		}
	}
	di->vol_gain = val;
	hwlog_info("vol_gain %d\n", di->vol_gain);

	return 0;
}

static struct coul_cali_ops bq27z561_cali_ops = {
	.dev_name = "aux",
	.get_current = bq27z561_get_calibration_curr,
	.get_voltage = bq27z561_get_calibration_vol,
	.set_current_gain = bq27z561_set_current_gain,
	.set_voltage_gain = bq27z561_set_voltage_gain,
	.set_cali_mode = bq27z561_enable_cali_mode,
};

static int bq27z561_calibration_para_invalid(int c_gain, int v_gain)
{
	return ((c_gain < BQ27Z561_TBATICAL_MIN_A) ||
		(c_gain > BQ27Z561_TBATICAL_MAX_A) ||
		(v_gain < BQ27Z561_TBATCAL_MIN_A) ||
		(v_gain > BQ27Z561_TBATCAL_MAX_A));
}

static void bq27z561_init_calibration_para(void)
{
	int c_a = BQ27Z561_CALI_PRECIE;
	int v_a = BQ27Z561_CALI_PRECIE;

	if (!g_bq27z561_dev)
		return;

	coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_CUR_A, &c_a);
	coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_VOL_A, &v_a);

	if (bq27z561_calibration_para_invalid(c_a, v_a)) {
		coul_cali_get_para(COUL_CALI_MODE_MAIN, COUL_CALI_PARA_CUR_A, &c_a);
		coul_cali_get_para(COUL_CALI_MODE_MAIN, COUL_CALI_PARA_VOL_A, &v_a);
		if (bq27z561_calibration_para_invalid(c_a, v_a)) {
			c_a = BQ27Z561_CALI_PRECIE;
			v_a = BQ27Z561_CALI_PRECIE;
			hwlog_info("all paras invalid use default val\n");
			goto update;
		}
	}

	hwlog_info("c_a %d, v_a %d\n", c_a, v_a);
update:
	bq27z561_set_voltage_gain(v_a, g_bq27z561_dev);
	bq27z561_set_current_gain(c_a, g_bq27z561_dev);
}

static void bq27z561_batt_para_check_work(struct work_struct *work)
{
	int dc;
	int ret;
	int chem_id;
	int version;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di null\n");
		return;
	}

	hwlog_info("[%s] batt_version = 0x%x, cfg_version = 0x%x\n",
		__func__, di->batt_version, di->fg_para_version);

	if (di->batt_version == di->fg_para_version) {
		hwlog_info("match version = 0x%x\n", di->batt_version);
		goto no_need_update;
	}
	atomic_set(&di->is_update, 1); /* 1: set flag */
	ret = bq27z561_update_bqfs_image();
	if (ret)
		hwlog_err("update bqfs image fail\n");

no_need_update:
	atomic_set(&di->is_update, 0); /* 0: clear flag */
	bq27z561_init_calibration_para();
	bq27z561_dump_registers(di);
	dc = bq27z561_read_dc();
	chem_id = bq27z561_get_chem_id();
	version = bq27z561_read_para_ver();
	hwlog_info("chem_id:0x%x, dc:0x%x, version:0x%x\n", chem_id, dc, version);
	schedule_delayed_work(&di->batt_para_monitor_work, msecs_to_jiffies(0));
}

static void bq27z561_batt_para_monitor_work(struct work_struct *work)
{
	u8 status = 0;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di)
		return;

	__pm_wakeup_event(&di->wakelock, jiffies_to_msecs(HZ));

	bq27z561_get_battery_info(di);
	di->batt_status = bq27z561_get_batt_status(di);
	di->control_status = bq27z561_read_control_status();
	di->op_status = bq27z561_read_op_status();
	di->charing_status = bq27z561_read_charging_status();
	di->gauging_status = bq27z561_read_gauging_status();
	di->manu_status = bq27z561_read_manu_status();
	bq27z561_read_byte(BQ27Z561_INT_STATUS_REG, &status);

	if (di->wakelock.active)
		__pm_relax(&di->wakelock);

	hwlog_info("fcc%d, rsoc%d, volt%d, current%d, current_avg%d\t"
		"temperature%d, batt_rm%d, batt_status0x%x, control_status0x%x\t"
		"op_status0x%x, charing_status0x%x, gauging_status0x%x\t"
		"manu_status0x%x, status:0x%x, qmax:%d\n",
		di->batt_fcc, di->batt_soc, di->batt_volt, di->batt_curr,
		di->batt_curr_avg, di->batt_temp, di->batt_rm, di->batt_status,
		di->control_status, di->op_status, di->charing_status,
		di->gauging_status, di->manu_status, status, di->batt_qmax);

	schedule_delayed_work(&di->batt_para_monitor_work,
		msecs_to_jiffies(5000)); /* 5s */
}

static int bq27z561_rom_mode(void)
{
	u8 cmd[2] = { 0x00, 0x0f }; /* rom mode cmd */

	/* change to rom mode */
	if (bq27z561_write_block(FG_FLASH_MODE_I2C_ADDR, 0, cmd, 2)) {
		hwlog_err("change to rom mode error\n");
		return -FG_ERR_I2C_W;
	}

	return 0;
}

static int bq27z561_get_parameters(char *buf, long int *param, int num_of_par)
{
	int cnt;
	int base;
	char *token = NULL;

	token = strsep(&buf, " ");

	for (cnt = 0; cnt < num_of_par; cnt++) {
		if (token) {
			if ((token[1] == 'x') || (token[1] == 'X'))
				base = 16; /* Hexadecimal */
			else
				base = 10; /* Decimal */

			if (kstrtol(token, base, &param[cnt]) != 0)
				return -EINVAL;

			token = strsep(&buf, " ");
		} else {
			return -EINVAL;
		}
	}

	return 0;
}

static ssize_t bq27z561_show_attrs(struct device *,
	struct device_attribute *, char *);
static ssize_t bq27z561_store_attrs(struct device *,
	struct device_attribute *, const char *, size_t count);

#define bq27z561_attr(_name) \
{ \
	.attr = { .name = #_name, .mode = 0664 }, \
	.show = bq27z561_show_attrs, \
	.store = bq27z561_store_attrs, \
}

static struct device_attribute bq27z561_fuelgauge_attrs[] = {
	bq27z561_attr(flash_mode),
	bq27z561_attr(rom_mode),
	bq27z561_attr(curr_gain),
	bq27z561_attr(vol_gain),
};

enum {
	FG_FLASH_MODE = 0,
	FG_ROM_MODE,
	CURR_GAIN,
	VOL_GAIN,
};

static ssize_t bq27z561_show_attrs(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int cnt = 0;
	u8 t_buf[4] = { 0 }; /* 4: data field size */
	const ptrdiff_t offset = attr - bq27z561_fuelgauge_attrs;
	struct bq27z561_device_info *di = g_bq27z561_dev;

	if (!di) {
		hwlog_err("di null\n");
		return -FG_ERR_PARA_NULL;
	}

	switch (offset) {
	case FG_FLASH_MODE:
		cnt += scnprintf(buf + cnt, PAGE_SIZE - cnt, "%d\n",
			di->batt_mode);
		break;
	case FG_ROM_MODE:
		cnt += scnprintf(buf + cnt, PAGE_SIZE - cnt, "%d\n",
			di->batt_mode);
		break;
	case CURR_GAIN:
		/* 4: CC GAIN cmd len */
		bq27z561_read_mac_data(FG_MAC_CMD_CC_GAIN, t_buf, 4);

		cnt += scnprintf(buf + cnt, PAGE_SIZE - cnt, "%02X%02X%02X%02X\n",
			t_buf[0], t_buf[1], t_buf[2], t_buf[3]);
		break;
	case VOL_GAIN:
		/* 4: CELL GAIN cmd len */
		bq27z561_read_mac_data(FG_MAC_CMD_CELL_GAIN, t_buf, 4);

		cnt += scnprintf(buf + cnt, PAGE_SIZE - cnt, "%02X%02X%02X%02X\n",
			t_buf[0], t_buf[1], t_buf[2], t_buf[3]);
		break;
	default:
		cnt = -EINVAL;
		break;
	}

	return cnt;
}

static ssize_t bq27z561_store_attrs(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int i;
	int ret;
	long int val;
	long int vals[4] = { 0 }; /* 4: input walue buff */
	u8 t_buf[4] = { 0 }; /* 4: data size */
	struct bq27z561_device_info *di = g_bq27z561_dev;
	const ptrdiff_t offset = attr - bq27z561_fuelgauge_attrs;

	if (!di) {
		hwlog_err("di null\n");
		return -FG_ERR_PARA_NULL;
	}

	switch (offset) {
	case FG_FLASH_MODE:
		/* 1: parameters number */
		ret = bq27z561_get_parameters((char *)buf, &val, 1);
		if (ret < 0)
			ret = -EINVAL;
		if (bq27z561_flash_mode())
			ret = -EINVAL;
		else
			ret = count;
		hwlog_info("[%s] flash mode 0x%x\n", __func__, val);
		di->batt_mode = val;
		break;
	case FG_ROM_MODE:
		/* 1: parameters number */
		ret = bq27z561_get_parameters((char *)buf, &val, 1);
		if (ret < 0)
			ret = -EINVAL;
		if (bq27z561_rom_mode())
			ret = -EINVAL;
		else
			ret = count;
		di->batt_mode = val;
		hwlog_info("[%s] rom mode 0x%x\n", __func__, val);
		break;
	case CURR_GAIN:
		/* 4: number of parameters */
		ret = bq27z561_get_parameters((char *)buf, vals, 4);
		if (ret < 0)
			ret = -EINVAL;
		for (i = 0; i < 4; i++)
			t_buf[i] = vals[i];

		/* 4: CC GAIN cmd len */
		bq27z561_write_mac_data(FG_MAC_CMD_CC_GAIN, t_buf, 4);

		hwlog_info("[%s] CURR_GAIN\n", __func__);
		ret = count;
		break;
	case VOL_GAIN:
		/* 2: parameters num */
		ret = bq27z561_get_parameters((char *)buf, vals, 2);
		if (ret < 0)
			ret = -EINVAL;
		for (i = 0; i < 2; i++)
			t_buf[i] = vals[i];

		/* 2: CELL GAIN cmd len */
		bq27z561_write_mac_data(FG_MAC_CMD_CELL_GAIN, t_buf, 2);
		hwlog_info("[%s] VOL_GAIN\n", __func__);
		ret = count;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int bq27z561_create_attrs(struct device *dev)
{
	int i;
	int rc = 0;

	for (i = 0; i < ARRAY_SIZE(bq27z561_fuelgauge_attrs); i++) {
		rc = device_create_file(dev, &bq27z561_fuelgauge_attrs[i]);
		if (rc)
			goto create_attrs_failed;
	}
	goto create_attrs_succeed;

create_attrs_failed:
	dev_notice(dev, "%s: failed %d\n", __func__, rc);
	while (i--)
		device_remove_file(dev, &bq27z561_fuelgauge_attrs[i]);
create_attrs_succeed:
	return rc;
}

static irqreturn_t bq27z561_irq_thread(int irq, void *dev_id)
{
	int ret;
	u8 status;
	int retry_cnt;
	struct bq27z561_device_info *di = dev_id;

	if (atomic_read(&di->is_update)) {
		hwlog_info("irq_thread is updating fw, ignore irq\n");
		return IRQ_HANDLED;
	}
	/* 10: wait resume */
	for (retry_cnt = 0; retry_cnt < 10; retry_cnt++) {
		if (atomic_read(&di->pm_suspend)) {
			hwlog_info("irq_thread wait resume\n");
			msleep(50); /* 50: wait resume */
		} else {
			break;
		}
	}
	if (retry_cnt >= 10) /* 10: wait resume */
		return  IRQ_HANDLED;

	bq27z561_read_status(di);
	bq27z561_dump_registers(di);
	bq27z561_get_battery_info(di);
	hwlog_info("RSOC:%d, Volt:%d, Current:%d, Current_avg:%d, Temperature:%d\n",
		di->batt_soc, di->batt_volt, di->batt_curr,
		di->batt_curr_avg, di->batt_temp);
	hwlog_info("irq_thread\n");
	/* InterruptStatus Reg */
	ret = bq27z561_read_byte(BQ27Z561_INT_STATUS_REG, &status);
	if (!ret) {
		/* 0x01: Interrupt Status, VOLT_HI */
		hwlog_info("VOLT_HI %s\n", status & 0x01 ? "set" : "clear");
		/* 0x04: Interrupt Status, TEMP_HI */
		hwlog_info("TEMP_HI %s\n", status & 0x04 ? "set" : "clear");
		/* 0x02: Interrupt Status, VOLT_LOW */
		hwlog_info("VOLT_LOW %s\n", status & 0x02 ? "set" : "clear");
		/* 0x08: Interrupt Status, TEMP_LOW */
		hwlog_info("TEMP_LOW %s\n", status & 0x08 ? "set" : "clear");
	}

	return IRQ_HANDLED;
}

static int bq27z561_irq_init(struct bq27z561_device_info *di)
{
	int ret;

	if (power_gpio_config_interrupt(di->dev->of_node,
		"bq27z561,gpio-intb", "bq27z561_int", &di->gpio, &di->irq))
		return -EINVAL;

	ret = devm_request_threaded_irq(di->dev, di->irq, NULL,
		bq27z561_irq_thread,
		IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
		"bq_fuel_gauge_irq", di);
	if (ret)
		return ret;

	enable_irq_wake(di->irq);
	device_init_wakeup(di->dev, true);
	hwlog_info("irq_init ok\n");

	return 0;
}

static int bq27z561_irq_chip_init(struct bq27z561_device_info *di)
{
	int ret;
	u8 status = 0;
	u8 io_cfg = FG_IO_CONFIG_VAL;
	u8 d_soc = BQ27Z561_SOC_DELTA_TH_VAL;

	ret = bq27z561_write_block(INVALID_REG_ADDR, BQ27Z561_SOC_DELTA_TH_REG,
		&d_soc, sizeof(d_soc));
	ret += bq27z561_write_word(BQ27Z561_VOLT_LOW_SET_REG,
		BQ27Z561_VOLT_LOW_SET_VAL);
	ret += bq27z561_write_word(BQ27Z561_VOLT_LOW_CLR_REG,
		BQ27Z561_VOLT_LOW_CLR_VAL);
	ret += bq27z561_write_mac_data(FG_MAC_CMD_IO_CONFIG,
		&io_cfg, sizeof(io_cfg));

	/* clear int status */
	ret += bq27z561_read_byte(BQ27Z561_INT_STATUS_REG, &status);
	ret += bq27z561_irq_init(di);

	return ret;
}

static int bq27z561_verify_para_ver(struct bq27z561_device_info *di)
{
	int ret = 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -FG_ERR_PARA_NULL;
	}

	di->batt_version = bq27z561_read_para_ver();
	if (di->batt_version == -FG_ERR_I2C_R) {
		di->batt_version = FG_PARA_INVALID_VER;
		ret = bq27z561_flash_mode();
	}
	if (ret) {
		hwlog_err("verify fail\n");
		return -FG_ERR_PARA_WRONG;
	}

	return 0;
}

static int bq27z561_parse_fg_para(struct device *dev,
	struct bq27z561_device_info *di)
{
	int ret;
	bool choose_default_flag = false;
	struct device_node *dev_node = NULL;
	struct device_node *np = dev->of_node;
	const char *batt_default_name = NULL;
	const char *batt_name = bat_model_name();

	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), np,
		"batt_default_name", &batt_default_name))
		return -EINVAL;

	if (!batt_name || (batt_name[0] == 0)) {
		batt_name = batt_default_name;
		choose_default_flag = true;
	}

	dev_node = of_find_node_by_name(np, batt_name);
	if (!dev_node && !choose_default_flag)
		dev_node = of_find_node_by_name(np, batt_default_name);
	if (!dev_node) {
		hwlog_err("batt para read failed\n");
		return -EINVAL;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), dev_node,
		"fg_para_version", (u32 *)&di->fg_para_version,
		FG_PARA_INVALID_VER);
	hwlog_info("fg_para_version = 0x%x\n", di->fg_para_version);

	di->count = of_property_count_u8_elems(dev_node, "bqfs_image_data");
	if (di->count <= 0)
		return -EINVAL;

	hwlog_info("count = %d\n", di->count);
	di->bqfs_image_data = devm_kzalloc(dev, sizeof(u8) * di->count,
		GFP_KERNEL);
	if (!di->bqfs_image_data)
		return -ENOMEM;

	ret = power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), dev_node,
		"bqfs_image_data", di->bqfs_image_data, di->count);
	if (ret) {
		devm_kfree(dev, di->bqfs_image_data);
		di->bqfs_image_data = NULL;
	}

	return ret;
}

static void bq27z561_parse_batt_ntc(struct device_node *np,
	struct bq27z561_device_info *pdata)
{
	int array_len;
	int i;
	long idata = 0;
	const char *string = NULL;
	int ret;

	if (!np)
		return;
	if (of_property_read_u32(np, "ntc_compensation_is",
		&(pdata->ntc_compensation_is))) {
		hwlog_info("get ntc_compensation_is failed\n");
		return;
	}
	array_len = of_property_count_strings(np, "ntc_temp_compensation_para");
	if ((array_len <= 0) || (array_len % NTC_PARA_TOTAL != 0)) {
		hwlog_err("ntc is invaild,please check ntc_temp_para number\n");
		return;
	}
	if (array_len > NTC_PARA_LEVEL * NTC_PARA_TOTAL) {
		array_len = NTC_PARA_LEVEL * NTC_PARA_TOTAL;
		hwlog_err("temp is too long use only front %d paras\n", array_len);
		return;
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np,
			"ntc_temp_compensation_para", i, &string);
		if (ret) {
			hwlog_err("get ntc_temp_compensation_para failed\n");
			return;
		}
		/* 10 means decimalism */
		ret = kstrtol(string, 10, &idata);
		if (ret)
			break;

		switch (i % NTC_PARA_TOTAL) {
		case NTC_PARA_ICHG:
			pdata->ntc_temp_compensation_para[i / NTC_PARA_TOTAL]
				.refer = idata;
			break;
		case NTC_PARA_VALUE:
			pdata->ntc_temp_compensation_para[i / NTC_PARA_TOTAL]
				.comp_value = idata;
			break;
		default:
			hwlog_err("ntc_temp_compensation_para get failed\n");
		}
		hwlog_info("ntc_temp_compensation_para[%d][%d] = %ld\n",
			i / (NTC_PARA_TOTAL), i % (NTC_PARA_TOTAL), idata);
	}
}

static int bq27z561_parse_dts(struct device *dev,
	struct bq27z561_device_info *di)
{
	int ret;

	bq27z561_parse_batt_ntc(dev->of_node, di);

	ret = bq27z561_parse_fg_para(dev, di);
	if (ret) {
		hwlog_err("parse_dts: parse fg_para failed\n");
		return ret;
	}

	return 0;
}

static int bq27z561_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct bq27z561_device_info *di = NULL;

	hwlog_info("probe begin\n");

	if (!client || !client->dev.of_node || !id)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_bq27z561_dev = di;
	di->dev = &client->dev;
	di->client = client;
	di->chip = id->driver_data;
	i2c_set_clientdata(client, di);

	if (di->chip != BQ27Z561) {
		hwlog_err("unexpected fuel gauge: %d\n", di->chip);
		goto dev_check_fail;
	}

	ret = bq27z561_parse_dts(&client->dev, di);
	if (ret)
		goto parse_dts_fail;

	memcpy(di->regs, bq27z561_regs, NUM_REGS);
	bq27z561_create_attrs(di->dev);

	wakeup_source_init(&di->wakelock, "fuel_gauge_chip");
	mutex_init(&di->rd_mutex);
	mutex_init(&di->mac_mutex);

	ret = bq27z561_verify_para_ver(di);
	if (ret)
		goto verify_para_ver_fail;

	di->last_batt_soc = bq27z561_read_last_capacity();
	bq27z561_get_battery_info(di);
	ret = bq27z561_irq_chip_init(di);
	if (ret)
		goto init_irq_fail;

	bq27z561_read_fw_version();
	coul_interface_ops_register(&bq27z561_ops);
	bq27z561_fg_ops.dev_data = (void *)di;
	power_log_ops_register(&bq27z561_fg_ops);
	coul_cali_ops_register(&bq27z561_cali_ops);
	INIT_DELAYED_WORK(&di->batt_para_check_work,
		bq27z561_batt_para_check_work);
	schedule_delayed_work(&di->batt_para_check_work,
		msecs_to_jiffies(0));
	INIT_DELAYED_WORK(&di->batt_para_monitor_work,
		bq27z561_batt_para_monitor_work);
	hwlog_info("bq27z561 probe successfully, %s\n", device2str[di->chip]);

	return 0;

verify_para_ver_fail:
init_irq_fail:
	wakeup_source_trash(&di->wakelock);
	mutex_destroy(&di->rd_mutex);
	mutex_destroy(&di->mac_mutex);
parse_dts_fail:
dev_check_fail:
	devm_kfree(&client->dev, di);
	g_bq27z561_dev = NULL;
	return ret;
}

static int bq27z561_remove(struct i2c_client *client)
{
	struct bq27z561_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return 0;

	wakeup_source_trash(&di->wakelock);
	mutex_destroy(&di->rd_mutex);
	mutex_destroy(&di->mac_mutex);
	free_irq(di->irq, di);
	gpio_free(di->gpio);
	cancel_delayed_work(&di->batt_para_monitor_work);
	cancel_delayed_work(&di->batt_para_check_work);
	devm_kfree(&client->dev, di);
	g_bq27z561_dev = NULL;

	return 0;
}

static int bq27z561_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq27z561_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return 0;

	cancel_delayed_work(&di->batt_para_monitor_work);
	atomic_set(&di->pm_suspend, 1); /* 1: set flag */

	return 0;
}

static int bq27z561_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bq27z561_device_info *di = i2c_get_clientdata(client);

	if (!di)
		return 0;
	atomic_set(&di->pm_suspend, 0);
	schedule_delayed_work(&di->batt_para_monitor_work,
		msecs_to_jiffies(0));

	return 0;
}

static const struct dev_pm_ops bq27z561_pm_ops = {
	.suspend = bq27z561_suspend,
	.resume = bq27z561_resume,
};

MODULE_DEVICE_TABLE(i2c, bq27z561);
static const struct of_device_id bq27z561_of_match[] = {
	{
		.compatible = "huawei,bq27z561",
		.data = NULL,
	},
	{},
};

static const struct i2c_device_id bq27z561_id[] = {
	{ "bq27z561", BQ27Z561 }, {},
};

static struct i2c_driver bq27z561_driver = {
	.probe = bq27z561_probe,
	.remove = bq27z561_remove,
	.id_table = bq27z561_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "bq27z561_fg",
		.of_match_table = of_match_ptr(bq27z561_of_match),
		.pm = &bq27z561_pm_ops,
	},
};

module_i2c_driver(bq27z561_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("bq27z561 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
