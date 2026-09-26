/*
 * lt9711a.c
 *
 * operate function for lt9711a
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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
#include <securec.h>
#include "edp_bridge.h"

#define MAX_CHIP_PROBE_TIMES 3

#define I2C_CONTROLLER     4
#define I2C_TRANSFER_LEN   2

#define CHIP_ID_REG_0    0x00
#define CHIP_ID_REG_1    0x01
#define MAX_RETRIES 10
#define EDID_READY_RET_1 0xee
#define EDID_READY_RET_2 0x11
#define FIRST_CIRCLE 1
#define msg_to_choose(addr,rw,i2c,mot,addr_only,len) (addr<<16 | rw<<11 | i2c<<10 | mot<<9 | addr_only<<8 | len)

static int is_lt9711a_onboard(struct mipi2edp *pdata)
{
	int i, ret, reg, chipid;

	HISI_DRM_INFO("check whether lt9711a chip is onboard\n");
	ret = 0;
	reg = 0;
	chipid = 0;
	for (i = 0; i < MAX_CHIP_PROBE_TIMES; i++) {
		ret = regmap_read(pdata->regmap, CHIP_ID_REG_0, &reg);
		if (ret)
			continue;
		else
			chipid = (reg&0xFF) << 8;

		ret = regmap_read(pdata->regmap, CHIP_ID_REG_1, &reg);
		if (ret)
			continue;
		else
			chipid |= (reg&0xFF);

		HISI_DRM_INFO("lt9711a is onboard,chipid is 0x%x\n", chipid);
		return ret;
	}

	HISI_DRM_ERR("lt9711a isn't onboard!\n");
	return ret;
}

static int lt9711a_i2c_send_byte(char devaddr, const char regoffset, const char value)
{
	struct i2c_adapter *adap;
	int ret;
	char data[I2C_TRANSFER_LEN];
	struct i2c_msg msg[1];
	int retries = 3;
	unsigned char xfers = XFER_ONE;

	adap = i2c_get_adapter(I2C_CONTROLLER);
	if (!adap) {
		HISI_DRM_ERR("lt9711a_i2c_send_byte i2c_get_adapter err");
		return -ENODEV;
	}

	msg->addr = devaddr;
	msg->flags = 0;
	msg->len = I2C_TRANSFER_LEN;
	msg->buf = data;
	data[0] = regoffset;
	data[1] = value;

	do {
		ret = i2c_transfer(adap, msg, xfers);
	} while (ret != xfers && --retries);

	HISI_DRM_DEBUG("lt9711a_i2c_send_byte regoffset = %d value=%d ret = %d \n", regoffset, value, ret);
	i2c_put_adapter(adap);
	if (ret == 1) {
		return 0;
	}
	return ret;
}

static int lt9711a_i2c_recv_byte(char devaddr, const char regoffset, char *value)
{
	struct i2c_adapter *adap;
	int ret;
	struct i2c_msg msg[2];
	int retries = 3;
	unsigned char xfers = XFER_TWO;

	adap = i2c_get_adapter(I2C_CONTROLLER);
	if (!adap) {
		HISI_DRM_ERR("lt9711a_i2c_recv_byte i2c_get_adapter err");
		return -ENODEV;
	}

	msg[0].addr = devaddr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = (uint8_t *)&regoffset;

	msg[1].addr = devaddr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = value;
	do {
		ret = i2c_transfer(adap, msg, xfers);
	} while (ret != xfers && --retries);

	HISI_DRM_DEBUG("lt9711a_i2c_send_byte regoffset=%d value=%d ret = %d \n", regoffset,value[0],ret);
	i2c_put_adapter(adap);
	if (ret == 2) {
		return 0;
	}
	return ret;
}

static int lt9711a_get_edid(char *edid, u32 len)
{
	char ret[2] = {0};
	int res;
	int top = 0;
	int i = 0;
	int j = 0;
	int base_address = 0;
	int data_register = 0xd8;
	int current_data = 0;
	int try_times = 0;

	HISI_DRM_INFO("+");
	if (!edid) {
		HISI_DRM_ERR("edid input err");
		return -1;
	}

	if (len < 128) {
		HISI_DRM_ERR("len input err");
		return -1;
	}
	/* lt9711a need times to run it's firmware, and wait until it's internal
	 * edid ready.
	 */
	mdelay(250);
	while (true) {
		res = lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A, 0xff, 0x80);
		if (res) {
			mdelay(20);
			try_times ++;
			HISI_DRM_ERR("lt9711a_i2c_transfer err 1 try_times:%d\n", try_times);
		} else {
			HISI_DRM_INFO("lt9711a_i2c_transfer res=%d\n", res);
			break;
		}
		if (try_times >= MAX_RETRIES) {
			HISI_DRM_ERR("lt9711a_i2c_transfer err 1. try max times");
			return -1;
		}
	}

	/*
	 * When edid of lt9711a is ready,lt9711a chip will write value 0xee or ox11 to the register 0x80d6.
	 * Getting edid for the first time,we need to judge the value of register 0x80d6.
	 * While getting edid,the value of register 0x80d6 will be overwrite,
	 * so getting edid not for the first time,wo can't judge the value of register 0x80d6.
	 */
	try_times = 0;
	while (true) {
		res = lt9711a_i2c_recv_byte(CHIP_I2C_ADDR_LT9711A, 0xd6, ret);
		if (res || ((ret[0] != EDID_READY_RET_1) && (ret[0] != EDID_READY_RET_2))) {
			mdelay(40);
			try_times++;
			HISI_DRM_ERR("lt9711a_i2c_recv_byte err 2! ret[0]=%x,try_times:%d\n", ret[0], try_times);
		} else {
			HISI_DRM_INFO("lt9711a_i2c_recv_byte ret[0]=%x,res=%d\n", ret[0], res);
			break;
		}

		if (try_times >= MAX_RETRIES) {
			HISI_DRM_ERR("lt9711a_i2c_recv_byte err 2. try max times");
			return -1;
		}
	}

	res = lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A, 0xe0, 0x20);
	if (res) {
		HISI_DRM_ERR("lt9711a_i2c_transfer err 3");
		return -1;
	}

	for(i = 0; i < 16; i++) {
		res = lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A, 0xe1, base_address);
		if (res) {
			HISI_DRM_ERR("lt9711a_i2c_transfer err 4");
			return -1;
		}
		mdelay(50);
		for(j = 0; j < 8; j++) {
			current_data = data_register + j;
			res = lt9711a_i2c_recv_byte(CHIP_I2C_ADDR_LT9711A, current_data, ret);
			if (res) {
				HISI_DRM_ERR("lt9711a_i2c_transfer err 5");
				return -1;
			}
			edid[top++] = ret[0];
		}

		base_address = base_address + 8;
	}
	if (top != len) {
		HISI_DRM_ERR("edid len err");
		return -1;
	}
	res = lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A, 0xe0, 0x40);
	if (res)
		HISI_DRM_WARN("lt9711a_i2c_transfer warn");
	res = lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A, 0xe1, 0x00);
	if (res)
		HISI_DRM_WARN("lt9711a_i2c_transfer warn");
	HISI_DRM_INFO("EDID:\n");
	print_hex_dump(KERN_NOTICE,
		" \t", DUMP_PREFIX_NONE, 16, 1,
			edid, EDID_LEN, false);
	HISI_DRM_INFO("-");
	return 0;
}

static int get_choose_from_msg(struct drm_dp_aux_msg *msg)
{
	int choose = CHOOSE_UNKNOWN;
	bool rw = false;
	bool addr_only = false;
	bool i2c = true;
	bool mot = false;
	u32 len;
	int ret;
	int i;

	HISI_DRM_INFO("[+]\n");
	if (msg == NULL)
		return choose;

	len = msg->size;
	if (msg->size == 0 && !msg->buffer) {
		addr_only = true;
		len = 1;
	}
	mot = (msg->request & DP_AUX_I2C_MOT) ? true : false;

	switch (msg->request & ~DP_AUX_I2C_MOT) {
	case DP_AUX_NATIVE_WRITE:
		i2c = false;					//no need break
	case DP_AUX_I2C_WRITE:
	case DP_AUX_I2C_WRITE_STATUS_UPDATE:
		rw = false;
		break;
	case DP_AUX_NATIVE_READ:
		i2c = false;					//no need break
	case DP_AUX_I2C_READ:
		rw = true;
		break;
	default:
		HISI_DRM_ERR("invalid msg request\n");
		ret = -EINVAL;
		return ret;
	}

	//rw=0---write        rw=1---read
	if (rw == 0) {
		u8 *bufx = (u8 *)msg->buffer;
		HISI_DRM_INFO("msg->address=%x msg->request=%d msg->reply=%d msg->size_t=%lu \n    ",msg->address,msg->request,msg->reply,msg->size);
		for (i = 0; i < msg->size; i++)
			HISI_DRM_INFO("%x ",bufx[i]);
	} else {
		HISI_DRM_INFO("msg->address=%x msg->request=%d msg->reply=%d msg->size_t=%lu \n    ",msg->address,msg->request,msg->reply,msg->size);
	}

	choose = msg_to_choose(msg->address,rw,i2c,mot,addr_only,len);
	HISI_DRM_INFO("[-]\n");
	return choose;
}


/*
 * copy edid from edid_array to buf
 * the length of buf is len
 * the length of edid_array is src_len
 * the function return the length of right copy
 */
static int lt9711_copy_edid(u8 *buf, int len, u8 *edid_array, int src_len)
{
	int count;

	if (!buf || !edid_array) {
		HISI_DRM_ERR("input err\n");
		return 0;
	}

	if (len != VCP_BUFFER_LENGTH) {
		HISI_DRM_ERR("invalid len:%d", len);
		return 0;
	}

	if (len > src_len) {
		HISI_DRM_ERR("len is larger than src_len");
		return 0;
	}

	HISI_DRM_INFO("start copy EDID\n");

	for (count = 0; count < len; count++) {
		buf[count] = edid_array[count];
		HISI_DRM_INFO("buf=%x\n", buf[count]);
	}

	HISI_DRM_INFO("end copy EDID\n");
	return count;
}

static void lt9711_set_luminance(u8 light_value)
{
	HISI_DRM_INFO("start setvcp\n");
	//send the command
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xff,0x80);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe1,0x6e);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe2,0x51);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe3,0x84);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe4,0x03);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe5,0x10);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe6,0x00);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe7,light_value);
	HISI_DRM_INFO("light_value=%d\n",light_value);

	//execute the command
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe8,0x33);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe0,0x01);

	mdelay(10);		//must delay

	//clear register
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe0,0x00);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe1,0x00);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe2,0x00);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe3,0x00);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe4,0x00);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe5,0x00);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe6,0x00);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe7,0x00);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe8,0x00);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe0,0x00);
	HISI_DRM_INFO("end setvcp\n");
}

static void lt9711_get_luminance(u8 *buf,int len)
{
	u8 bufferaddr = 0xd1;
	int count;
	if (len != VCP_BUFFER_LENGTH) {
		HISI_DRM_INFO("invalid len:%d", len);
		return;
	}

	HISI_DRM_INFO("start getvcp\n");
	//send the command
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xff,0x80);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe1,0x6e);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe2,0x51);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe3,0x82);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe4,0x01);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe5,0x10);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe6,0xac);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe0,0x01);
	mdelay(50);			//at least delay 50ms

	//read result and write result to register
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe0,0x22);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xff,0x80);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe1,0x6f);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe2,0x6e);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe3,0x88);
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe0,0x01);
	mdelay(10);			//at least delay 10ms

	//read result from register to buf
	for (count = 0; count < VCP_BUFFER_LENGTH - 1; count++) {
		lt9711a_i2c_recv_byte(CHIP_I2C_ADDR_LT9711A,bufferaddr,&buf[count]);
		HISI_DRM_INFO("buf=%x\n",buf[count]);
		bufferaddr += 1;
	}
	buf[VCP_BUFFER_LENGTH - 1] = 0x00;
	mdelay(10);

	//clear register
	lt9711a_i2c_send_byte(CHIP_I2C_ADDR_LT9711A,0xe0,0x00);

	HISI_DRM_INFO("end getvcp\n");
}

static void lt9711_response_getvcp(int getvcp_choose,struct drm_dp_aux_msg *msg)
{
	u8 *buf = NULL;
	if (!msg || !msg->buffer || msg->size != VCP_BUFFER_LENGTH) {
		HISI_DRM_ERR("parame err\n");
		return;
	}
	buf = (u8 *)(msg->buffer);

	if (getvcp_choose == VCP_OPCODE_FOR_LUMINANCE) {
		lt9711_get_luminance(buf,msg->size);
	} else if (getvcp_choose == VCP_OPCODE_FOR_VCPVERSION) {
		u8 vcp_version_array[VCP_BUFFER_LENGTH] = {0x6e,0x88,0x02,0x00,0xdf,0x00,0xff,0xff,0x02,0x02,0x6b,0x00,0x00,0x00,0x00,0x00};
		memcpy_s(buf,msg->size,vcp_version_array,msg->size);
	} else if (getvcp_choose == NO_DATA) {
		u8 vcp_code_page_buff[VCP_BUFFER_LENGTH] = {0x6e,0x88,0x02,0x01,0x00,0x00,0xff,0xff,0x01,0x20,0x94,0x00,0x00,0x00,0x00,0x00};
		memcpy_s(buf,msg->size,vcp_code_page_buff,msg->size);
	}
}

static void lt9711_get_vcp_opcode(int *getvcp_choose,u8 *buf)
{
	if (!getvcp_choose || !buf) {
		HISI_DRM_ERR("parame err\n");
		return;
	}

	HISI_DRM_INFO("change getvcp_choose\n");
	if (buf[VCP_OPCODE_POS] == VCP_OPCODE_FOR_VCPVERSION) {
		*getvcp_choose = VCP_OPCODE_FOR_VCPVERSION;
	} else if (buf[VCP_OPCODE_POS] == VCP_OPCODE_FOR_LUMINANCE) {
		*getvcp_choose = VCP_OPCODE_FOR_LUMINANCE;
	} else if (buf[VCP_OPCODE_POS] == NO_DATA) {
		*getvcp_choose = NO_DATA;
	} else {
		HISI_DRM_INFO("unknown vcp choose\n");
		*getvcp_choose = UNKNOWN_VCP_OPCODE;
	}
}

static ssize_t lt9711_aux_transfer(struct drm_dp_aux *aux,struct drm_dp_aux_msg *msg)
{
	u8 *buf = (u8 *)(msg->buffer);
	u8 bytes[1];
	u8 light_value;
	static int getvcp_choose = UNKNOWN_VCP_OPCODE;
	int choose = CHOOSE_UNKNOWN;
	struct mipi2edp *pdata = container_of(aux, struct mipi2edp, aux);
	static int edid_pointer = 0;

	HISI_DRM_INFO("[+]\n");
	mutex_lock(&pdata->lock);
	if (!aux || !msg) {
		HISI_DRM_ERR("parameter is null\n");
		mutex_unlock(&pdata->lock);
		return -1;
	}

	choose = get_choose_from_msg(msg);
	if (choose < 0) {
		HISI_DRM_INFO("[-]\n");
		mutex_unlock(&pdata->lock);
		return choose;
	}

	if (msg->size == 0 && !msg->buffer)
		buf = bytes;

	HISI_DRM_INFO("choose=%x getvcp_choose=%d\n ",choose,getvcp_choose);
	switch (choose) {
	case CHOOSE_GET_EDID:
		edid_pointer += lt9711_copy_edid(buf, msg->size, pdata->edid_array + edid_pointer, EDID_LENGTH - edid_pointer);
		break;

	case CHOOSE_RESPONSE_NODATA_1:
	case CHOOSE_RESPONSE_NODATA_2:
	case CHOOSE_RESPONSE_NODATA_3:
	case CHOOSE_RESPONSE_NODATA_4:
	case CHOOSE_RESPONSE_NODATA_5:
		HISI_DRM_ERR("set buf=0\n");
		memset_s(buf,msg->size,0,msg->size);
		break;

	case CHOOSE_EDID_ADDR:
		HISI_DRM_ERR("set buf[0]=2\n");
		buf[0] = 0x02;
		edid_pointer = 0;
		break;

	case CHOOSE_SET_LUMINANCE:
		light_value = buf[LIGHT_VALUE_POS];
		lt9711_set_luminance(light_value);
		break;

	case CHOOSE_GET_VCP_OPCODE:
		lt9711_get_vcp_opcode(&getvcp_choose,buf);
		HISI_DRM_INFO("now getvcp_choose=%x\n",getvcp_choose);
		break;

	case CHOOSE_GET_ADDR:
		buf[0] = LT9711A_I2C_LUMINANCE_ADDR << 1;
		HISI_DRM_ERR("set buf[0]=addr<<1\n");
		break;

	case CHOOSE_RESPONSE_GETVCP:
		lt9711_response_getvcp(getvcp_choose,msg);
		getvcp_choose = UNKNOWN_VCP_OPCODE;
		break;

	default:
		HISI_DRM_ERR("do nothing for choose(%x)\n",choose);
		break;
	}

	if (edid_pointer >= EDID_LENGTH)
		edid_pointer = 0;

	mutex_unlock(&pdata->lock);
	HISI_DRM_INFO("[-]\n");
	return msg->size;
}

static struct regmap_config lt9711a_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.reg_stride = 1,
};

static struct mipi2edp_bridge_chip_info lt9711a_bridge_chip_info = {
	.chip_addr = CHIP_I2C_ADDR_LT9711A,
	.name = "lt9711-aux",
	.regmap_config = &lt9711a_regmap,
	.is_chip_onboard = is_lt9711a_onboard,
	.pre_enable = NULL,
	.enable = NULL,
	.disable = NULL,
	.post_disable = NULL,
	.get_edid = lt9711a_get_edid,
	.transfer = lt9711_aux_transfer,
	.set_clk = NULL
};

static int __init lt9711a_chip_register(void)
{
	int ret = 0;

	printk(KERN_DEBUG "lt9711a_chip_register +");

	ret = mipi2edp_bridge_chip_info_register(&lt9711a_bridge_chip_info);

	printk(KERN_DEBUG "lt9711a_chip_register -");

	return ret;
}

fs_initcall(lt9711a_chip_register);

