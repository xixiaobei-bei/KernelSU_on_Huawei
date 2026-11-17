/*
 * adapter_protocol_uvdm.c
 *
 * uvdm protocol driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/delay.h>
#include <chipset_common/hwpower/protocol/adapter_protocol.h>
#include <chipset_common/hwpower/protocol/adapter_protocol_uvdm.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG uvdm_protocol
HWLOG_REGIST();

static struct uvdm_protocol_dev *g_uvdm_protocol_dev;

static const struct adapter_protocol_device_data g_uvdm_protocol_dev_data[] = {
	{ PROTOCOL_DEVICE_ID_SCHARGER_V600, "scharger_v600" },
};

static struct uvdm_protocol_dev *uvdm_protocol_get_dev(void)
{
	if (!g_uvdm_protocol_dev) {
		hwlog_err("g_uvdm_protocol_dev is null\n");
		return NULL;
	}

	return g_uvdm_protocol_dev;
}

static struct uvdm_protocol_ops *uvdm_protocol_get_ops(void)
{
	if (!g_uvdm_protocol_dev || !g_uvdm_protocol_dev->p_ops) {
		hwlog_err("g_pd_protocol_dev or p_ops is null\n");
		return NULL;
	}

	return g_uvdm_protocol_dev->p_ops;
}

static int uvdm_protocol_get_device_id(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_uvdm_protocol_dev_data); i++) {
		if (!strncmp(str, g_uvdm_protocol_dev_data[i].name,
			strlen(str)))
			return g_uvdm_protocol_dev_data[i].id;
	}

	return -1;
}

int uvdm_protocol_ops_register(struct uvdm_protocol_ops *ops)
{
	int dev_id;

	if (!g_uvdm_protocol_dev || !ops || !ops->chip_name) {
		hwlog_err("g_uvdm_protocol_dev or ops or chip_name is null\n");
		return -1;
	}

	dev_id = uvdm_protocol_get_device_id(ops->chip_name);
	if (dev_id < 0) {
		hwlog_err("%s ops register fail\n", ops->chip_name);
		return -1;
	}

	g_uvdm_protocol_dev->p_ops = ops;
	g_uvdm_protocol_dev->dev_id = dev_id;

	hwlog_info("%d:%s ops register ok\n", dev_id, ops->chip_name);
	return 0;
}

static u32 uvdm_protocol_package_header_data(u32 cmd)
{
	return (UVDM_CMD_DIRECTION_INITIAL << UVDM_HDR_SHIFT_CMD_DIRECTTION) |
		(cmd << UVDM_HDR_SHIFT_CMD) |
		(UVDM_FUNCTION_DC_CTRL << UVDM_HDR_SHIFT_FUNCTION) |
		(UVDM_VERSION << UVDM_HDR_SHIFT_VERSION) |
		(UVDM_VDM_TYPE << UVDM_HDR_SHIFT_VDM_TYPE) |
		(UVDM_VID << UVDM_HDR_SHIFT_VID);
}

static int uvdm_protocol_send_data(u32 *data, u8 cnt, bool wait_rsp)
{
	int i;
	struct uvdm_protocol_ops *l_ops = NULL;

	l_ops = uvdm_protocol_get_ops();
	if (!l_ops)
		return -1;

	if (!l_ops->send_data) {
		hwlog_err("send_data is null\n");
		return -1;
	}

	if ((cnt <= UVDM_VDOS_COUNT_BEGIN) || (cnt >= UVDM_VDOS_COUNT_END)) {
		hwlog_err("invalid vdos count=%d\n", cnt);
		return -1;
	}

	for (i = 0; i < cnt; i++)
		hwlog_info("data[%d] = %2x\n", i, data[i]);

	l_ops->send_data(data, cnt, wait_rsp, l_ops->dev_data);
	return 0;
}

static int uvdm_protocol_handle_vdo_data(u32 *data, int len,
	bool response, unsigned int retrys)
{
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();
	int ret;
	unsigned int retry_count = 0;

	if (!l_dev)
		return -1;

RETRY:
	/* false: need response */
	ret = uvdm_protocol_send_data(data, len, false);
	if (ret)
		return -1;

	if (!response)
		return 0;

	ret = wait_for_completion_timeout(&l_dev->rsp_comp,
		msecs_to_jiffies(UVDM_WAIT_RESPONSE_TIME));
	reinit_completion(&l_dev->rsp_comp);
	if (!ret) {
		hwlog_err("wait response timeout\n");
		if (retry_count++ < retrys)
			goto RETRY;

		return -1;
	}

	return 0;
}

static int uvdm_protocol_set_output_voltage(int volt)
{
	u32 data[UVDM_VDOS_COUNT_TWO] = { 0 };

	/* data[0]: header */
	data[0] = uvdm_protocol_package_header_data(UVDM_CMD_SET_VOLTAGE);
	/* data[1]: voltage */
	data[1] = volt / UVDM_VOLTAGE_UNIT;

	return uvdm_protocol_handle_vdo_data(data, UVDM_VDOS_COUNT_TWO, false, 0);
}

static int uvdm_protocol_get_output_voltage(int *volt)
{
	u32 data[UVDM_VDOS_COUNT_ONE] = { 0 };
	int ret;
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();

	if (!l_dev || !volt)
		return -1;

	/* data[0]: header */
	data[0] = uvdm_protocol_package_header_data(UVDM_CMD_GET_VOLTAGE);

	ret = uvdm_protocol_handle_vdo_data(data, UVDM_VDOS_COUNT_ONE, true,
		UVDM_RETRY_TIMES);
	if (ret)
		return -1;

	*volt = l_dev->info.volt;
	return 0;
}

static int uvdm_protocol_get_adp_type(int *type)
{
	int ret;
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();

	if (!l_dev || !type)
		return -1;

	ret = wait_for_completion_timeout(&l_dev->report_type_comp,
		msecs_to_jiffies(UVDM_WAIT_RESPONSE_TIME));
	reinit_completion(&l_dev->report_type_comp);
	if (!ret) {
		hwlog_err("get_adp_type wait response timeout\n");
		return -1;
	}

	*type = l_dev->info.power_type;
	return 0;
}

static int uvdm_protocol_set_encrypt_index(u8 *data, int index)
{
	if (!data)
		return -1;

	data[UVDM_KEY_INDEX_OFFSET] = index;

	hwlog_info("set_encrypt_index: %d\n", index);
	return 0;
}

static int uvdm_protocol_set_random_num(u8 *data, int start, int end)
{
	int i;

	if (!data)
		return -1;

	for (i = start; i <= end; i++)
		get_random_bytes(&data[i], sizeof(u8));

	return 0;
}

static int uvdm_protocol_send_random_num(u8 *random)
{
	u32 data[UVDM_VDOS_COUNT_THREE] = { 0 };

	if (!random)
		return -1;

	/* data[0]: header */
	data[0] = uvdm_protocol_package_header_data(UVDM_CMD_SEND_RANDOM);
	/* data[1],data[2]: random value */
	data[1] = (random[UVDM_VDO_BYTE_ZERO] <<
		(UVDM_VDO_BYTE_ZERO * UVDM_RANDOM_HASH_SHIFT)) |
		(random[UVDM_VDO_BYTE_ONE] <<
		(UVDM_VDO_BYTE_ONE * UVDM_RANDOM_HASH_SHIFT)) |
		(random[UVDM_VDO_BYTE_TWO] <<
		(UVDM_VDO_BYTE_TWO * UVDM_RANDOM_HASH_SHIFT)) |
		(random[UVDM_VDO_BYTE_THREE] <<
		(UVDM_VDO_BYTE_THREE * UVDM_RANDOM_HASH_SHIFT));
	data[2] = (random[UVDM_VDO_BYTE_FOUR] <<
		(UVDM_VDO_BYTE_ZERO * UVDM_RANDOM_HASH_SHIFT)) |
		(random[UVDM_VDO_BYTE_FIVE] <<
		(UVDM_VDO_BYTE_ONE * UVDM_RANDOM_HASH_SHIFT)) |
		(random[UVDM_VDO_BYTE_SIX] <<
		(UVDM_VDO_BYTE_TWO * UVDM_RANDOM_HASH_SHIFT)) |
		(random[UVDM_VDO_BYTE_SEVEN] <<
		(UVDM_VDO_BYTE_THREE * UVDM_RANDOM_HASH_SHIFT));

	return uvdm_protocol_handle_vdo_data(data, UVDM_VDOS_COUNT_THREE, true,
		UVDM_RETRY_TIMES);
}

static int uvdm_protocol_get_encrypted_value(void)
{
	u32 data[UVDM_VDOS_COUNT_ONE] = { 0 };

	/* data[0]: header */
	data[0] = uvdm_protocol_package_header_data(UVDM_CMD_GET_HASH);

	return uvdm_protocol_handle_vdo_data(data, UVDM_VDOS_COUNT_ONE, true,
		UVDM_RETRY_TIMES);
}

static uvdm_protocol_copy_hash_value(u8 *hash, int len)
{
	int i;
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();

	for (i = 0; i < UVDM_ENCRYPT_RANDOM_LEN; i++)
		hash[i] = l_dev->encrypt_random_value[i];

	for (i = 0; i < UVDM_ENCTYPT_HASH_LEN; i++)
		hash[i + UVDM_ENCRYPT_RANDOM_LEN] = l_dev->encrypt_hash_value[i];

	return 0;
}

static int uvdm_protocol_auth_encrypt_start(int key, u8 *hash, int size)
{
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();

	if (!hash || !l_dev)
		return -1;

	/* first: set key index */
	if (uvdm_protocol_set_encrypt_index(l_dev->encrypt_random_value, key))
		return -1;

	/* second: host create random num */
	if (uvdm_protocol_set_random_num(l_dev->encrypt_random_value,
		UVDM_RANDOM_S_OFFSET, UVDM_RANDOM_E_OFFESET))
		return -1;

	/* third: host set random num to slave */
	if (uvdm_protocol_send_random_num(l_dev->encrypt_random_value))
		return -1;

	/* fourth: host get hash num from slave */
	if (uvdm_protocol_get_encrypted_value())
		return -1;

	/* fifth: copy hash value */
	if (uvdm_protocol_copy_hash_value(hash, size))
		return -1;

	hwlog_info("auth encrypt start\n");
	return 0;
}

static int uvdm_protocol_set_slave_power_mode(int mode)
{
	u32 data[UVDM_VDOS_COUNT_TWO] = { 0 };

	/* data[0]: header */
	data[0] = uvdm_protocol_package_header_data(UVDM_CMD_SWITCH_POWER_MODE);
	/* data[1]: power mode */
	data[1] = mode;

	if (mode == UVDM_PWR_MODE_BUCK2SC)
		return uvdm_protocol_handle_vdo_data(data, UVDM_VDOS_COUNT_TWO,
			true, UVDM_RETRY_TIMES);
	else
		return uvdm_protocol_handle_vdo_data(data, UVDM_VDOS_COUNT_TWO,
			false, 0);
}

/* reduce the voltage of rx when charge done */
static int uvdm_protocol_set_rx_reduce_voltage(void)
{
	u32 data[UVDM_VDOS_COUNT_ONE] = { 0 };

	/* data[0]: header */
	data[0] = uvdm_protocol_package_header_data(UVDM_CMD_SET_RX_REDUCE_VOLTAGE);

	return uvdm_protocol_handle_vdo_data(data, UVDM_VDOS_COUNT_ONE,
		true, UVDM_RETRY_TIMES);
}

static int uvdm_protocol_get_protocol_register_state(void)
{
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();

	if (!l_dev)
		return -1;

	if ((l_dev->dev_id >= PROTOCOL_DEVICE_ID_BEGIN) &&
		(l_dev->dev_id < PROTOCOL_DEVICE_ID_END))
		return 0;

	hwlog_info("get_protocol_register_state fail\n");
	return -1;
}

static struct adapter_protocol_ops adapter_protocol_uvdm_ops = {
	.type_name = "hw_uvdm",
	.set_output_voltage = uvdm_protocol_set_output_voltage,
	.get_output_voltage = uvdm_protocol_get_output_voltage,
	.auth_encrypt_start = uvdm_protocol_auth_encrypt_start,
	.set_slave_power_mode = uvdm_protocol_set_slave_power_mode,
	.set_rx_reduce_voltage = uvdm_protocol_set_rx_reduce_voltage,
	.get_adp_type = uvdm_protocol_get_adp_type,
	.get_protocol_register_state = uvdm_protocol_get_protocol_register_state,
};

static int uvdm_protocol_check_receive_data(u32 data)
{
	struct uvdm_protocol_header_data hdr;

	hwlog_info("uvdm_header: 0x%02x\n", data);

	hdr.vdm_type = (data >> UVDM_HDR_SHIFT_VDM_TYPE) & UVDM_MASK_VDM_TYPE;
	hdr.vid = (data >> UVDM_HDR_SHIFT_VID) & UVDM_MASK_VID;

	if ((hdr.vdm_type == UVDM_VDM_TYPE) && (hdr.vid == UVDM_VID))
		return 0;

	hwlog_err("invalid uvdm header\n");
	return -1;
}

static void uvdm_protocol_report_power_type(u32 data)
{
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();

	if (!l_dev)
		return;

	l_dev->info.power_type = data & UVDM_MASK_POWER_TYPE;
	complete(&l_dev->report_type_comp);
}

static void uvdm_protocol_send_random(u32 data)
{
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();

	if (!l_dev)
		return;

	if ((data & UVDM_MASK_RESPONSE_STATE) == UVDM_RESPONSE_TX_INTERRUPT)
		complete(&l_dev->rsp_comp);
}

static void uvdm_protocol_switch_power_mode(u32 data)
{
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();

	if (!l_dev)
		return;

	hwlog_info("switch power mode data = %x\n", data);
	if ((data & UVDM_MASK_RESPONSE_STATE) == UVDM_RESPONSE_POWER_READY)
		complete(&l_dev->rsp_comp);
}

static void uvdm_protocol_get_voltage(u32 data)
{
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();

	if (!l_dev)
		return;

	l_dev->info.volt = (data & UVDM_MASK_VOLTAGE) * UVDM_VOLTAGE_UNIT;
	complete(&l_dev->rsp_comp);
}

static void uvdm_protocol_get_hash(u32 *data, int len)
{
	int i, j, k;
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();

	if (!l_dev)
		return;

	if (len > UVDM_VDOS_COUNT_THREE)
		return;

	/* data[1],data[2]: hash values */
	for (i = 0; i < UVDM_ENCTYPT_HASH_LEN; i++) {
		k = i / UVDM_VDO_BYTE_FOUR + 1; /* k = {1, 2} */
		j = i % UVDM_VDO_BYTE_FOUR * UVDM_RANDOM_HASH_SHIFT;
		l_dev->encrypt_hash_value[i] = data[k] >> j & UVDM_MASK_KEY;
	}
	complete(&l_dev->rsp_comp);
}

static void uvdm_protocol_report_otg_event(u32 data)
{
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();

	if (!l_dev)
		return;

	l_dev->info.otg_event = data & UVDM_MASK_OTG_EVENT;
	power_event_anc_notify(POWER_ANT_UVDM_FAULT,
		POWER_NE_UVDM_FAULT_OTG, &l_dev->info.otg_event);
}

static void uvdm_protocol_report_abnormal_event(u32 data)
{
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();

	if (!l_dev)
		return;

	l_dev->info.abnormal_flag = data & UVDM_MASK_REPORT_ABNORMAL;
	power_event_anc_notify(POWER_ANT_UVDM_FAULT,
		POWER_NE_UVDM_FAULT_COVER_ABNORMAL, &l_dev->info.abnormal_flag);
}

static void uvdm_protocol_handle_receive_dc_crtl_data(u32 *vdo, int len)
{
	u32 cmd;
	u32 vdo_hdr;
	u32 vdo_data0;

	vdo_hdr = vdo[0]; /* vdo[0]: uvdm header */
	vdo_data0 = vdo[1]; /* vdo[1]: the first data of uvdm data */

	cmd = (vdo_hdr >> UVDM_HDR_SHIFT_CMD) & UVDM_MASK_CMD;
	switch (cmd) {
	case UVDM_CMD_REPORT_POWER_TYPE:
		uvdm_protocol_report_power_type(vdo_data0);
		break;
	case UVDM_CMD_REPORT_ABNORMAL:
		uvdm_protocol_report_abnormal_event(vdo_data0);
		break;
	case UVDM_CMD_SEND_RANDOM:
		uvdm_protocol_send_random(vdo_data0);
		break;
	case UVDM_CMD_SWITCH_POWER_MODE:
		uvdm_protocol_switch_power_mode(vdo_data0);
		break;
	case UVDM_CMD_GET_VOLTAGE:
		uvdm_protocol_get_voltage(vdo_data0);
		break;
	case UVDM_CMD_GET_HASH:
		uvdm_protocol_get_hash(vdo, UVDM_VDOS_COUNT_THREE);
		break;
	default:
		break;
	}
}

static void uvdm_protocol_handle_receive_pd_crtl_data(u32 *vdo, int len)
{
	u32 cmd;
	u32 vdo_hdr;
	u32 vdo_data0;

	vdo_hdr = vdo[0]; /* vdo[0]: uvdm header */
	vdo_data0 = vdo[1]; /* vdo[1]: the first data of uvdm data */

	cmd = (vdo_hdr >> UVDM_HDR_SHIFT_CMD) & UVDM_MASK_CMD;
	switch (cmd) {
	case UVDM_CMD_REPORT_OTG_EVENT:
		uvdm_protocol_report_otg_event(vdo_data0);
		break;
	default:
		break;
	}
}

static int uvdm_protocol_handle_receive_data(void *data)
{
	u32 vdo[UVDM_VDOS_COUNT_SEVEN] = { 0 };
	u32 vdo_hdr;
	u32 func;
	int ret;

	if (!data)
		return -1;

	memcpy(vdo, data, sizeof(vdo));
	vdo_hdr = vdo[0]; /* vdo[0]: uvdm header */
	ret = uvdm_protocol_check_receive_data(vdo_hdr);
	if (ret)
		return -1;

	func = (vdo_hdr >> UVDM_HDR_SHIFT_FUNCTION) & UVDM_MASK_FUNCTION;
	switch (func) {
	case UVDM_FUNCTION_DC_CTRL:
		uvdm_protocol_handle_receive_dc_crtl_data(vdo, UVDM_VDOS_COUNT_THREE);
		break;
	case UVDM_FUNCTION_PD_CTRL:
		uvdm_protocol_handle_receive_pd_crtl_data(vdo, UVDM_VDOS_COUNT_THREE);
		break;
	default:
		break;
	}

	return 0;
}

static int uvdm_protocol_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct uvdm_protocol_dev *l_dev = uvdm_protocol_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_UVDM_RECEIVE:
		uvdm_protocol_handle_receive_data(data);
		return NOTIFY_OK;
	default:
		return NOTIFY_OK;
	}
}

static int __init uvdm_protocol_init(void)
{
	int ret;
	struct uvdm_protocol_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_uvdm_protocol_dev = l_dev;
	l_dev->dev_id = PROTOCOL_DEVICE_ID_END;

	init_completion(&l_dev->rsp_comp);
	init_completion(&l_dev->report_type_comp);
	l_dev->nb.notifier_call = uvdm_protocol_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_UVDM, &l_dev->nb);
	if (ret)
		goto fail_free_mem;

	ret = adapter_protocol_ops_register(&adapter_protocol_uvdm_ops);
	if (ret)
		goto fail_unregister_notifier;

	return 0;

fail_unregister_notifier:
	power_event_bnc_unregister(POWER_BNT_UVDM, &l_dev->nb);
fail_free_mem:
	kfree(l_dev);
	g_uvdm_protocol_dev = NULL;
	return ret;
}

static void __exit uvdm_protocol_exit(void)
{
	if (!g_uvdm_protocol_dev)
		return;

	power_event_bnc_unregister(POWER_BNT_UVDM, &g_uvdm_protocol_dev->nb);
	kfree(g_uvdm_protocol_dev);
	g_uvdm_protocol_dev = NULL;
}

subsys_initcall_sync(uvdm_protocol_init);
module_exit(uvdm_protocol_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("uvdm protocol driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
