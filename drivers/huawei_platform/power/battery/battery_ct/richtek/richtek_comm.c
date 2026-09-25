/*
 * richtek_comm.c
 *
 * Richtek IC communication protocol.
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include "richtek_comm.h"
#include <linux/types.h>
#include <linux/pm_qos.h>
#include <linux/smp.h>
#include <linux/random.h>
#include <linux/list.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG hw_richtek_comm
HWLOG_REGIST();

static const unsigned char g_crc_table[] = {
	0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15,
	0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
	0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
	0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
	0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5,
	0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
	0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85,
	0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
	0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
	0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
	0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2,
	0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
	0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32,
	0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
	0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
	0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
	0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c,
	0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
	0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec,
	0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
	0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
	0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
	0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c,
	0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
	0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b,
	0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
	0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
	0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
	0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb,
	0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
	0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb,
	0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3,
};
static struct pm_qos_request g_pm_qos;

static const struct rt_cmd g_rt_cmds[] = {
	{ RT_READ_STATUS,     2,  8,  0,  0, "read status" },
	{ RT_READ_CMD_CRC,    1,  0,  0,  0, "read cmd crc" },
	{ RT_AUTH_WI_ROM_ID,  0,  0,  15, 0, "auth" },
	{ RT_GEN_WORK_SECRET, 0,  0,  15, 0, "gen work secret" },
	{ RT_WRITE_CHALLENGE, 32, 0,  1,  0, "write challenge" },
	{ RT_READ_MAC,        32, 0,  0,  0, "read device id" },
	{ RT_READ_ROM_ID,     8,  8,  0,  0, "read rom id" },
	{ RT_LOCK_MFA_PAGE1,  0,  8,  15, 0, "lock mfa page1" },
	{ RT_LOCK_MFA_PAGE2,  0,  8,  15, 0, "lock mfa page1" },
	{ RT_UNSEAL_SYS,      2,  8,  15, 0, "unseal sys" },
	{ RT_SEAL_NVM,        0,  8,  15, 0, "seal nvm" },
	{ RT_WRITE_MFA_PAGE1, 18, 8,  75, 0, "write mfa page1" },
	{ RT_WRITE_MFA_PAGE2, 4,  8,  15, 0, "write mfa page2" },
	{ RT_WRITE_SYS_PAGE,  12, 8,  45, 0, "read sys page" },
	{ RT_WRITE_SYS_PWD,   2,  8,  15, 0, "write sys pwd" },
	{ RT_READ_MFA_PAGE1,  18, 8,  0,  0, "read mfa page1" },
	{ RT_READ_MFA_PAGE2,  4,  8,  0,  0, "read mfa page2" },
	{ RT_READ_SYS_PAGE,   12, 8,  0,  0, "read sys page" },
	{ RT_READ_DEVICE_ID,  4,  8,  0,  0, "read device id" },
	{ RT_INC_CYC_CNT,     0,  8,  0,  0, "inc cyc" },
	{ RT_READ_CYC_CN,     2,  8,  15, 0, "read cyc" },
};

static unsigned char rt_calc_crc8(unsigned char *data, unsigned int length)
{
	unsigned char crc8 = 0;
	int i;

	for (i = 0; i < length; i++)
		crc8 = g_crc_table[crc8 ^ data[i]];

	return crc8;
}

static unsigned char rt_calc_cmd_crc8(unsigned int no,
	const unsigned char *data, unsigned int length)
{
	int i;
	int buf_cnt = 0;
	unsigned char buff[RT_MAX_BUF_LEN] = { 0 };

	buff[buf_cnt++] = (no & 0xFF00) >> RT_BIT_PER_BYTE;
	buff[buf_cnt++] = no & 0x00FF;

	for (i = 0; i < length; i++)
		buff[buf_cnt++] = data[i];

	return rt_calc_crc8(buff, buf_cnt);
}

static const struct rt_cmd *rt_get_cmd(struct rt_com_obj *obj, unsigned int no)
{
	int i;

	for (i = 0; i < obj->cmd_nums; i++) {
		if (no == obj->cmd_info[i].no)
			return &obj->cmd_info[i];
	}

	return NULL;
}

static void rt_incr_com_totals(struct rt_com_obj *obj, unsigned int no)
{
	int i;

	for (i = 0; i < obj->cmd_nums; i++) {
		if (no != obj->cmd_info[i].no)
			continue;

		if (obj->com_stat.totals)
			obj->com_stat.totals[i]++;
		return;
	}
}

static void rt_incr_com_errs(struct rt_com_obj *obj, unsigned int no)
{
	int i;

	for (i = 0; i < obj->cmd_nums; i++) {
		if (no != obj->cmd_info[i].no)
			continue;

		if (obj->com_stat.errs)
			obj->com_stat.errs[i]++;
		return;
	}

	if (no == RT_READ_CMD_CRC)
		return;

	obj->com_stat.cerrs_cnt++;
	if (obj->com_stat.cerrs_cnt > obj->com_stat.max_cerrs) {
		obj->com_stat.max_cerrs = obj->com_stat.cerrs_cnt;
		hwlog_err("continuous error %x:%x\n",
			obj->com_stat.max_cerrs, no);
	}
}

static void rt_clear_cerrs_cnt(struct rt_com_obj *obj)
{
	obj->com_stat.cerrs_cnt = 0;
}

static int rt_read_cmd(struct rt_com_obj *obj, const struct rt_cmd *cmds,
	unsigned char *rd_data, unsigned int length)
{
	unsigned char crc8;
	unsigned char clac_crc8;
	int k;
	int i;
	int ret = RT_COM_UNKNOWN_ERR;
	unsigned int retry = cmds->retry;
	struct ow_phy_ops *ops = &obj->phy_ops;

	if (length < cmds->len)
		return RT_COM_PARA_ERR;

	if (obj->sign & RT_COM_RETRY_NONE_MASK)
		retry = 1;

	for (k = 0; k < retry; k++) {
		rt_incr_com_totals(obj, cmds->no);

		ops->reset_write_byte((cmds->no & 0xFF00) >> RT_BIT_PER_BYTE);
		ops->write_byte(cmds->no & 0x00FF);
		for (i = 0; i < cmds->len; i++)
			rd_data[i] = ops->read_byte();

		crc8 = ops->read_byte();
		clac_crc8 = rt_calc_cmd_crc8(cmds->no, rd_data, cmds->len);
		if (crc8 != clac_crc8) {
			hwlog_err("crc8 check failed %x:%x in read 0x%x\n",
				crc8, clac_crc8, cmds->no);
			rt_incr_com_errs(obj, cmds->no);
			ret = RT_COM_CRC_ERR;
			continue;
		} else {
			ret = RT_COM_OK;
			rt_clear_cerrs_cnt(obj);
			break;
		}
	}

	if (!ret && (cmds->delay > 0))
		ops->wait_for_ic(cmds->delay);

	return ret;
}

static int rt_write_cmd_once(struct rt_com_obj *obj, const struct rt_cmd *cmds,
	unsigned char *wr_data, unsigned char wr_crc8)
{
	unsigned char rd_crc8;
	int i;
	int ret;
	const struct rt_cmd *rd_crc = rt_get_cmd(obj, RT_READ_CMD_CRC);
	struct ow_phy_ops *ops = &obj->phy_ops;

	rt_incr_com_totals(obj, cmds->no);
	ops->reset_write_byte((cmds->no & 0xFF00) >> RT_BIT_PER_BYTE);
	ops->write_byte(cmds->no & 0x00FF);
	for (i = 0; i < cmds->len; i++)
		ops->write_byte(wr_data[i]);
	ops->write_byte(wr_crc8);

	if (!rd_crc) {
		rt_clear_cerrs_cnt(obj);
		return RT_COM_OK;
	}

	ret = rt_read_cmd(obj, rd_crc, &rd_crc8, sizeof(rd_crc8));
	if (ret) {
		rt_incr_com_errs(obj, cmds->no);
		return ret;
	}

	if (rd_crc8 != wr_crc8) {
		hwlog_err("crc8 check failed in write command 0x%x\n",
			cmds->no);
		rt_incr_com_errs(obj, cmds->no);
		return RT_COM_CRC_ERR;
	}

	rt_clear_cerrs_cnt(obj);
	return RT_COM_OK;
}

static int rt_write_cmd(struct rt_com_obj *obj, const struct rt_cmd *cmds,
	unsigned char *wr_data)
{
	unsigned char wr_crc8;
	int k;
	int ret;
	unsigned int retry = cmds->retry;

	if (obj->sign & RT_COM_RETRY_NONE_MASK)
		retry = 1;

	wr_crc8 = rt_calc_cmd_crc8(cmds->no, wr_data, cmds->len);
	for (k = 0; k < retry; k++) {
		ret = rt_write_cmd_once(obj, cmds, wr_data, wr_crc8);
		if (ret == RT_COM_OK)
			break;
	}

	if (!ret && (cmds->delay > 0))
		obj->phy_ops.wait_for_ic(cmds->delay);

	return ret;
}

static int rt_check_phy_ops(struct ow_phy_ops *ops)
{
	if (!ops->reset || !ops->read_byte || !ops->reset_write_byte ||
		!ops->write_byte || !ops->set_time_request ||
		!ops->wait_for_ic) {
		hwlog_err("null:ops member found in %s\n", __func__);
		return -1;
	}

	return 0;
}

static int rt_read(struct rt_com_obj *obj, unsigned int no,
	unsigned char *data, unsigned int len)
{
	int ret;
	int cp_len;
	unsigned char tmp[RT_MAX_BUF_LEN] = { 0 };
	const struct rt_cmd *cmd = rt_get_cmd(obj, no);

	if (!obj || !data || !cmd)
		return RT_COM_PARA_ERR;
	if (cmd->len > RT_MAX_DATA_LEN) {
		hwlog_err("command 0x%x len out of limit %d\n", no, cmd->len);
		return RT_COM_PARA_ERR;
	}
	if (rt_check_phy_ops(&obj->phy_ops))
		return RT_COM_PHY_ERR;

	mutex_lock(&obj->com_lock);
	/* Add the QoS request and set the latency to 0 */
	pm_qos_add_request(&g_pm_qos, PM_QOS_CPU_DMA_LATENCY, 0);
	kick_all_cpus_sync();
	ret = rt_read_cmd(obj, cmd, tmp, RT_MAX_BUF_LEN);
	pm_qos_remove_request(&g_pm_qos);
	mutex_unlock(&obj->com_lock);
	if (ret) {
		hwlog_err("send read cmd 0x%x/%d error %d\n", no,
			cmd->len, ret);
		return ret;
	}

	cp_len = (len < cmd->len) ? len : cmd->len;
	memcpy(data, tmp, cp_len);
	hwlog_info("send read cmd 0x%x/%d success\n", no, cmd->len);

	return RT_COM_OK;
}

static int rt_write(struct rt_com_obj *obj, unsigned int no,
	unsigned char *data, unsigned int len)
{
	int ret;
	const struct rt_cmd *cmd = rt_get_cmd(obj, no);

	if (!obj || !cmd || ((len != 0) && !data))
		return RT_COM_PARA_ERR;
	if (cmd->len != len) {
		hwlog_err("command 0x%x len is error %d:%d\n", no,
			cmd->len, len);
		return RT_COM_PARA_ERR;
	}
	if (rt_check_phy_ops(&obj->phy_ops))
		return RT_COM_PHY_ERR;

	mutex_lock(&obj->com_lock);
	/* Add the QoS request and set the latency to 0 */
	pm_qos_add_request(&g_pm_qos, PM_QOS_CPU_DMA_LATENCY, 0);
	kick_all_cpus_sync();
	ret = rt_write_cmd(obj, cmd, data);
	pm_qos_remove_request(&g_pm_qos);
	mutex_unlock(&obj->com_lock);
	if (ret)
		hwlog_err("send write cmd 0x%x/%d error %d\n", no,
			cmd->len, ret);
	else
		hwlog_info("send write cmd 0x%x/%d success\n", no,
			cmd->len);

	return ret;
}

static void rt_close_com_retry(struct rt_com_obj *obj)
{
	if (obj)
		obj->sign |= RT_COM_RETRY_NONE_MASK;
}

static void rt_open_com_retry(struct rt_com_obj *obj)
{
	if (obj)
		obj->sign &= ~RT_COM_RETRY_NONE_MASK;
}

static int rt_read_one_shot(struct rt_com_obj *obj, unsigned int no,
	unsigned char *data, unsigned int len)
{
	int ret;

	if (!obj)
		return RT_COM_PARA_ERR;

	rt_close_com_retry(obj);
	ret = rt_read(obj, no, data, len);
	rt_open_com_retry(obj);
	return ret;
}

static int rt_write_one_shot(struct rt_com_obj *obj, unsigned int no,
	unsigned char *data, unsigned int len)
{
	int ret;

	if (!obj)
		return RT_COM_PARA_ERR;

	rt_close_com_retry(obj);
	ret = rt_write(obj, no, data, len);
	rt_open_com_retry(obj);
	return ret;
}

static int rt_get_com_stats(struct rt_com_obj *obj, char *buf,
	unsigned int size)
{
	int i;
	int val = 0;

	if (!obj || !obj->com_stat.totals || !obj->com_stat.errs)
		return snprintf(buf, size, "ERROR stat info\n");

	for (i = 0;  i < obj->cmd_nums;  i++) {
		if (obj->cmd_info && obj->cmd_info[i].str)
			val += snprintf(buf + val, size - val,
				"%s--total:%u, failed:%u times.\n",
				obj->cmd_info[i].str,
				obj->com_stat.totals[i],
				obj->com_stat.errs[i]);
		else
			val += snprintf(buf + val, size - val,
				"%d--total:%u, failed:%u times.\n", i,
				obj->com_stat.totals[i],
				obj->com_stat.errs[i]);
	}
	val += snprintf(buf + val, size - val,
		"all command cerrors--%u\n", obj->com_stat.max_cerrs);

	return val;
}

static void rt_reset_com_stats(struct rt_com_obj *obj)
{
	int size;

	if (!obj)
		return;

	size = obj->cmd_nums * sizeof(unsigned int);
	if (obj->com_stat.totals)
		memset(obj->com_stat.totals, 0, size);

	if (obj->com_stat.errs)
		memset(obj->com_stat.errs, 0, size);

	obj->com_stat.max_cerrs = 0;
	obj->com_stat.cerrs_cnt = 0;
}

static int rt_register_com_phy(struct rt_com_obj *obj, unsigned int phy_id)
{
	int ret = RT_COM_PHY_ERR;
	struct ow_phy_ops *ops = &obj->phy_ops;
	struct list_head *ow_phy_reg_head = get_owi_phy_list_head();
	struct ow_phy_list *pos = NULL;

	/* find the physical controller by matching phanlde number */
	list_for_each_entry(pos, ow_phy_reg_head, node) {
		if ((pos->onewire_phy_register != NULL) &&
			(phy_id == pos->dev_phandle))
			ret = pos->onewire_phy_register(ops);

		if (!ret) {
			hwlog_info("found phy-ctrl for richtek\n");
			break;
		}
	}
	if (ret) {
		hwlog_err("can't find physical controller for richtek\n");
		return ret;
	}

	/* check all onewire physical operations are valid */
	if (rt_check_phy_ops(ops))
		return RT_COM_PHY_ERR;

	/* set time request for physic controller first */
	ops->set_time_request(&obj->ow_trq);
	return RT_COM_OK;
}

static void rt_print_phy_time_rq(struct ow_treq *ow_trq)
{
	hwlog_info("timer frequence is %u, HZ is %u, loops per jiffy is %lu\n",
		arch_timer_get_cntfrq(), HZ, loops_per_jiffy);
	hwlog_info("%ucycles:%uns, %ucycles:%uns, %ucycles:%uns",
		ow_trq->reset_init_low_cycles, ow_trq->reset_init_low_ns,
		ow_trq->reset_slave_response_delay_cycles,
		ow_trq->reset_slave_response_delay_ns,
		ow_trq->reset_hold_high_cycles, ow_trq->reset_hold_high_ns);
	hwlog_info("%ucycles:%uns, %ucycles:%uns, %ucycles:%uns\n",
		ow_trq->write_init_low_cycles, ow_trq->write_init_low_ns,
		ow_trq->write_hold_cycles, ow_trq->write_hold_ns,
		ow_trq->write_residual_cycles, ow_trq->write_residual_ns);
	hwlog_info("%ucycles:%uns, %ucycles:%uns, %ucycles:%uns\n",
		ow_trq->read_init_low_cycles, ow_trq->read_init_low_ns,
		ow_trq->read_wait_slave_cycles, ow_trq->read_wait_slave_ns,
		ow_trq->read_residual_cycles, ow_trq->read_residual_ns);
}

static int rt_read_phy_trq_property(struct device_node *node, const char *name,
	unsigned int index, unsigned int *value)
{
	int ret;

	ret = of_property_read_u32_index(node, name, index, value);
	if (ret)
		hwlog_err("dts do not have %s/%d\n", name, index);

	return ret;
}

static int rt_phy_time_rq_init(struct ow_treq *ow_trq,
	struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;

	/* get 1-wire ic time requests */
	if (rt_read_phy_trq_property(node, "reset-time-request",
			FIRST_TIME_PROPERTY, &ow_trq->reset_init_low_ns) ||
		rt_read_phy_trq_property(node, "reset-time-request",
			SECOND_TIME_PROPERTY,
			&ow_trq->reset_slave_response_delay_ns) ||
		rt_read_phy_trq_property(node, "reset-time-request",
			THIRD_TIME_PROPERTY, &ow_trq->reset_hold_high_ns) ||
		rt_read_phy_trq_property(node, "write-time-request",
			FIRST_TIME_PROPERTY, &ow_trq->write_init_low_ns) ||
		rt_read_phy_trq_property(node, "write-time-request",
			SECOND_TIME_PROPERTY, &ow_trq->write_hold_ns) ||
		rt_read_phy_trq_property(node, "write-time-request",
			THIRD_TIME_PROPERTY, &ow_trq->write_residual_ns) ||
		rt_read_phy_trq_property(node, "read-time-request",
			FIRST_TIME_PROPERTY, &ow_trq->read_init_low_ns) ||
		rt_read_phy_trq_property(node, "read-time-request",
			SECOND_TIME_PROPERTY, &ow_trq->read_wait_slave_ns) ||
		rt_read_phy_trq_property(node, "read-time-request",
			THIRD_TIME_PROPERTY, &ow_trq->read_residual_ns) ||
		of_property_read_u32(node, "transport_bit_order",
			&ow_trq->transport_bit_order))
		return RT_COM_PHY_ERR;

	ow_trq->reset_hold_high_cycles = ns2cycles(ow_trq->reset_hold_high_ns);
	ow_trq->reset_init_low_cycles = ns2cycles(ow_trq->reset_init_low_ns);
	ow_trq->reset_slave_response_delay_cycles = ns2cycles(
		ow_trq->reset_slave_response_delay_ns);
	ow_trq->write_init_low_cycles = ns2cycles(ow_trq->write_init_low_ns);
	ow_trq->write_hold_cycles = ns2cycles(ow_trq->write_hold_ns);
	ow_trq->write_residual_cycles = ns2cycles(ow_trq->write_residual_ns);
	ow_trq->read_init_low_cycles = ns2cycles(ow_trq->read_init_low_ns);
	ow_trq->read_wait_slave_cycles = ns2cycles(ow_trq->read_wait_slave_ns);
	ow_trq->read_residual_cycles = ns2cycles(ow_trq->read_residual_ns);

	rt_print_phy_time_rq(ow_trq);
	return RT_COM_OK;
}

static void rt_com_ops_init(struct rt_comm_ops *ops)
{
	ops->read = rt_read;
	ops->write = rt_write;
	ops->read_one_shot = rt_read_one_shot;
	ops->write_one_shot = rt_write_one_shot;
	ops->get_stats = rt_get_com_stats;
	ops->reset_stats = rt_reset_com_stats;
	ops->register_com_phy = rt_register_com_phy;
}

static int rt_com_obj_init(struct rt_com_obj *obj, struct platform_device *pdev)
{
	struct rt_com_stat *com_stat = &obj->com_stat;

	obj->cmd_info = g_rt_cmds;
	obj->cmd_nums = ARRAY_SIZE(g_rt_cmds);

	com_stat->totals = devm_kzalloc(&pdev->dev,
		obj->cmd_nums * sizeof(int), GFP_KERNEL);
	if (!com_stat->totals)
		goto fail_mem;

	com_stat->errs = devm_kzalloc(&pdev->dev,
		obj->cmd_nums * sizeof(int), GFP_KERNEL);
	if (!com_stat->errs)
		goto fail_mem;

	mutex_init(&obj->com_lock);
	return RT_COM_OK;

fail_mem:
	devm_kfree(&pdev->dev, com_stat->totals);
	com_stat->totals = 0;
	devm_kfree(&pdev->dev, com_stat->errs);
	com_stat->errs = 0;
	return RT_COM_MEM_ERR;
}

int rt_comm_register(struct rt_ic_des *des, struct platform_device *pdev,
	unsigned int phy_id)
{
	int ret;
	struct rt_com_obj *obj = NULL;

	if (!des || !pdev)
		return RT_COM_PARA_ERR;
	obj = &des->obj;

	ret = rt_phy_time_rq_init(&obj->ow_trq, pdev);
	if (ret)
		return ret;

	rt_com_ops_init(&des->com_ops);
	return rt_com_obj_init(obj, pdev);
}

void rt_destroy_com_obj(struct rt_com_obj *obj, struct platform_device *pdev)
{
	if (!obj)
		return;

	devm_kfree(&pdev->dev, obj->com_stat.totals);
	obj->com_stat.totals = 0;
	devm_kfree(&pdev->dev, obj->com_stat.errs);
	obj->com_stat.errs = 0;

	mutex_destroy(&obj->com_lock);
}
