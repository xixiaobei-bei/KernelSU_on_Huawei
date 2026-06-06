/*
 * a1007.c
 *
 * this is for ic a1007.c
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include "a1007.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG a1007
HWLOG_REGIST();

#define GET_LOCK_STATUS_RETRY  8
#define GET_DEV_ID_RETRY       8
#define GET_READ_MEM_RETRY     8
#define GET_ECDH_RETRY         8
#define GET_VERYFY_MAC_RETRY   8
#define VALID_CERTZ_RETRY      8
#define GET_EEUNLOCK_RETRY     8
#define GET_EEWRT_LOCK_RETRY   8
#define GET_SN_RETRY           1
#define A1007_POWER_DOWN_RETRY 8
#define A1007_CERTZ_SEG_SIZE   32
#define A1007_LOCK_TIMEOUT     (5 * HZ)

static const uint32_t g_root_pubkey[] = {
	0xF45A3129, 0x252EA975, 0x784FDC1C, 0xCD56B5D6, 0xC8F3A5EF, 0x5ED2BE63,
	0x81309CEE, 0x1BFA94E8, 0x0687C5F6, 0x46D7EBCF, 0x2EDA79DB, 0x5B1C2DC2,
	0xEF916793, 0xFAACAAE2,
};

static const uint8_t g_org_name[] = {
	'8', '2', '1', '1', '2', '3', '5', '9', '0', '0',
};

static const uint8_t g_com_name[] = {
	'2', '1', '2', '0', '2', '0', '5', '1', '8', '2', '5', '0',
};

static void a1007_hard_reset(struct a1007_des *drv_data)
{
	struct ow_gpio_des *gpio = drv_data->gpio;
	struct ow_phy_ops_v2 *ops = &drv_data->ops;
	struct ow_treq_v2 *trq = &drv_data->trq;

	hwlog_info("%s hard reset\n", drv_data->name);
	ops->reset(gpio, trq);
	A1007_MEM_VALID(drv_data->mem.mac_context) = 0;
	A1007_MEM_VALID(drv_data->mem.lock_status) = 0;
}

static void a1007_soft_reset(struct a1007_des *drv_data)
{
	struct ow_gpio_des *gpio = drv_data->gpio;
	struct ow_phy_ops_v2 *ops = &drv_data->ops;
	struct ow_treq_v2 *trq = &drv_data->trq;

	hwlog_info("%s soft reset\n", drv_data->name);
	ops->soft_reset(gpio, trq);
}

static void a1007_mix_reset(struct a1007_des *drv_data, unsigned char num)
{
	if (num % 2)
		a1007_hard_reset(drv_data);
	else
		a1007_soft_reset(drv_data);
}

static int32_t a1007_owi_send(struct a1007_des *drv_data, const uint32_t len,
			      const uint8_t *buf)
{
	uint8_t addr = drv_data->addr;
	uint32_t i;
	struct ow_gpio_des *gpio = drv_data->gpio;
	struct ow_phy_ops_v2 *ops = &drv_data->ops;
	struct ow_treq_v2 *trq = &drv_data->trq;

	/* sometime bit signal pulse is wider, so send a stop bit first */
	if (ops->start(A1007_ADDR_WR(addr), gpio, trq)) {
		hwlog_err("Write Mode: Send addr(%02x) to ic failed\n", addr);
		return A1007_FAIL;
	}
	/* Write data */
	for (i = 0; i < len; i++) {
		if (ops->write_byte((i == len - 1), buf[i], gpio, trq)) {
			hwlog_err("Tx %uth data(%02x) failed\n", i, buf[i]);
			return A1007_FAIL;
		}
	}

	return A1007_SUCCESS;
}

/* buf need offer len bytes at least */
static int32_t __a1007_owi_recv(struct a1007_des *drv_data, const uint32_t len,
				const char get_crc, uint8_t *buf)
{
	uint8_t addr = drv_data->addr;
	uint8_t temp;
	uint8_t end;
	uint32_t i;
	uint8_t crc;
	struct ow_gpio_des *gpio = drv_data->gpio;
	struct ow_phy_ops_v2 *ops = &drv_data->ops;
	struct ow_treq_v2 *trq = &drv_data->trq;

	/* if want to read last bit of addr should be 1 */
	if (ops->start(A1007_ADDR_RD(addr), gpio, trq)) {
		hwlog_err("Read Mode: Send addr(%02x) to ic failed\n", addr);
		return A1007_FAIL;
	}
	/* read rx length */
	if (ops->read_byte(0, gpio, trq, buf)) {
		hwlog_err("Read data length from ic failed\n");
		return A1007_FAIL;
	}
	/* check rx length
	 * 0 means infinite to read
	 * 1 means only pcb
	 * if crc buf[0] == len - 1
	 * others data is accurate the size
	 */
	if (buf[0] == 0 || buf[0] == 1 || get_crc)
		crc = 0;
	else
		crc = 1;
	i = 0;
	if (buf[0] == (len - crc - 1) || buf[0] == 0 || buf[0] == 1) {
		/* Read PCB */
		if (ops->read_byte((i++ == len - 1), gpio, trq, buf + 1)) {
			hwlog_err("Read PCB from ic failed\n");
			return A1007_FAIL;
		}
		if (buf[0] == 1) {
			hwlog_info("PCB was %02x\n", buf[1]);
			if (len == 2)
				return A1007_SUCCESS;
			return A1007_FAIL;
		}
		if (buf[1] != 0) {
			hwlog_err("PCB error was %02x\n", buf[1]);
			ops->read_byte(1, gpio, trq, &temp);
			return buf[1];
		}
		for (i = A1007_DATA_HEAD_SIZE; i < len; i++) {
			end = (i == len - 1);
			if (ops->read_byte(end, gpio, trq, buf + i)) {
				hwlog_err("RX data(%uth) from ic failed\n", i);
				return A1007_FAIL;
			}
		}
	} else {
		ops->read_byte(1, gpio, trq, &temp);
		hwlog_err("IC says %d bytes, but %d bytes need\n", buf[0], len);
		return A1007_FAIL;
	}

	return A1007_SUCCESS;
}

static int32_t a1007_owi_recv(struct a1007_des *drv_data, const uint32_t len,
			      uint8_t *buf)
{
	return __a1007_owi_recv(drv_data, len, 0, buf);
}

static int32_t a1007_owi_recv_crc(struct a1007_des *drv_data,
				  const uint32_t len, uint8_t *buf)
{
	return __a1007_owi_recv(drv_data, len, 1, buf);
}

static int32_t __a1007_get_lstatus(struct a1007_des *drv_data)
{
	uint8_t rbuf[A1007_SML_BUFF_SIZE];
	uint32_t rlen;
	uint8_t wbuf[A1007_SML_BUFF_SIZE];
	uint32_t i = 0;

	wbuf[i++] = A1007_CMD_FIRST_BYTE(A1007_GET_LOCK_STATUS);
	wbuf[i] = (uint8_t)A1007_GET_LOCK_STATUS;
	rlen = A1007_GET_LSTATUS_LEN;

	if (a1007_owi_send(drv_data, i + 1, wbuf)) {
		hwlog_err("send get lock status cmd failed\n");
		return A1007_FAIL;
	}
	if (a1007_owi_recv(drv_data, rlen, rbuf)) {
		hwlog_err("recv get lock status data failed\n");
		return A1007_FAIL;
	}
	if (rbuf[rlen - 1] !=
	    a1007_crc8_calc(A1007_GET_LOCK_STATUS, rbuf, rlen - 1)) {
		hwlog_err("get lock status crc failed(%02x %02x %02x %02x)\n",
			  rbuf[0], rbuf[1], rbuf[2], rbuf[3]);
		return A1007_FAIL;
	}
	A1007_MEM_TO_DATA(drv_data->mem.lock_status)[0] = rbuf[2];

	return A1007_SUCCESS;
}

static int32_t _a1007_get_lstatus(struct a1007_des *drv_data)
{
	unsigned char retry;

	for (retry = 0; retry < GET_LOCK_STATUS_RETRY; retry++) {
		if (__a1007_get_lstatus(drv_data)) {
			hwlog_info("%s failed(%d@%d)", __func__, retry,
				   GET_LOCK_STATUS_RETRY);
			a1007_mix_reset(drv_data, retry);
		} else {
			return A1007_SUCCESS;
		}
	}

	return A1007_FAIL;
}

static int32_t a1007_get_lstatus(struct a1007_des *drv_data)
{
	if (A1007_MEM_VALID(drv_data->mem.lock_status))
		return A1007_SUCCESS;
	if (_a1007_get_lstatus(drv_data))
		return A1007_FAIL;
	A1007_MEM_VALID(drv_data->mem.lock_status) = 1;

	return A1007_SUCCESS;
}

static int32_t a1007_get_lstatus_noretry(struct a1007_des *drv_data)
{
	if (A1007_MEM_VALID(drv_data->mem.lock_status))
		return A1007_SUCCESS;
	if (__a1007_get_lstatus(drv_data)) {
		hwlog_err("__a1007_get_lstatus failed in %s for %s", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}
	A1007_MEM_VALID(drv_data->mem.lock_status) = 1;

	return A1007_SUCCESS;
}

/* this cmd can't be found in nxp a1007 data sheet v0.8 */
static int32_t __a1007_get_device_id(struct a1007_des *drv_data)
{
	uint8_t rbuf[A1007_SML_BUFF_SIZE];
	uint8_t wbuf[A1007_SML_BUFF_SIZE];
	uint8_t i;
	struct ow_gpio_des *gpio = drv_data->gpio;
	struct ow_phy_ops_v2 *ops = &drv_data->ops;
	struct ow_treq_v2 *trq = &drv_data->trq;

	wbuf[0] = A1007_DEVICE_ID_ADDR;
	wbuf[1] = drv_data->addr;

	a1007_hard_reset(drv_data);

	if (ops->start(A1007_ADDR_WR(wbuf[0]), gpio, trq)) {
		hwlog_err("read id failed at sending %02X\n",
			  A1007_ADDR_WR(wbuf[0]));
		return A1007_FAIL;
	}
	if (ops->write_byte(1, wbuf[1], gpio, trq)) {
		hwlog_err("read id failed at sending %02X\n", wbuf[1]);
		return A1007_FAIL;
	}
	if (ops->start(A1007_ADDR_RD(wbuf[0]), gpio, trq)) {
		hwlog_err("read id failed at sending %02X\n",
			  A1007_ADDR_RD(wbuf[0]));
		return A1007_FAIL;
	}
	for (i = 0; i < A1007_DEV_ID_BUFF_SIZE; i++) {
		if (ops->read_byte(((A1007_DEV_ID_BUFF_SIZE - 1) == i), gpio,
				   trq, rbuf + i)) {
			hwlog_err("read id failed at recv %uth byte\n", i + 1);
			return A1007_FAIL;
		}
	}

	if (rbuf[0]) {
		hwlog_err("illegal device id:%02x %02x %02x\n",
			  rbuf[0], rbuf[1], rbuf[2]);
		return A1007_FAIL;
	}
	/* ?also can't find in data sheet */
	if (rbuf[1] != 0x06) {
		hwlog_err("illegal device id:%02x %02x %02x\n",
			  rbuf[0], rbuf[1], rbuf[2]);
		return A1007_FAIL;
	}
	A1007_MEM_TO_DATA(drv_data->mem.dev_id)[0] = rbuf[0];
	A1007_MEM_TO_DATA(drv_data->mem.dev_id)[1] = rbuf[1];
	A1007_MEM_TO_DATA(drv_data->mem.dev_id)[2] = rbuf[2];
	hwlog_info("dev id: %02x %02x %02x", rbuf[0], rbuf[1], rbuf[2]);

	return A1007_SUCCESS;
}

static int32_t _a1007_get_device_id(struct a1007_des *drv_data)
{
	unsigned char retry;

	for (retry = 0; retry < GET_DEV_ID_RETRY; retry++) {
		if (__a1007_get_device_id(drv_data)) {
			hwlog_info("%s failed(%d@%d)", __func__, retry,
				   GET_DEV_ID_RETRY);
			a1007_mix_reset(drv_data, retry);
		} else {
			return A1007_SUCCESS;
		}
	}

	return A1007_FAIL;
}

static int32_t a1007_get_device_id(struct a1007_des *drv_data)
{
	if (A1007_MEM_VALID(drv_data->mem.dev_id))
		return A1007_SUCCESS;
	if (_a1007_get_device_id(drv_data))
		return A1007_FAIL;
	A1007_MEM_VALID(drv_data->mem.dev_id) = 1;

	return A1007_SUCCESS;
}

static int32_t a1007_get_crc(struct a1007_des *drv_data, uint8_t *crc)
{
	uint8_t rbuf[A1007_SML_BUFF_SIZE];
	uint32_t rlen;
	uint8_t wbuf[A1007_SML_BUFF_SIZE];
	uint32_t wlen;

	wbuf[0] = A1007_CMD_FIRST_BYTE(A1007_GET_CRC);
	wbuf[1] = (uint8_t)A1007_GET_CRC;
	wlen = 2;
	rlen = A1007_GET_CRC_LEN;

	if (a1007_owi_send(drv_data, wlen, wbuf)) {
		hwlog_err("send get crc cmd failed\n");
		return A1007_FAIL;
	}
	if (a1007_owi_recv_crc(drv_data, rlen, rbuf)) {
		hwlog_err("recv get crc data failed\n");
		return A1007_FAIL;
	}
	*crc = rbuf[rlen - 1];

	return A1007_SUCCESS;
}

static int32_t __a1007_read_memory(struct a1007_des *drv_data,
				   const uint16_t mem_addr,
				   const uint32_t len, uint8_t *buf)
{
	uint8_t rbuf[A1007_BIG_BUFF_SIZE];
	uint32_t rlen;
	uint8_t wbuf[A1007_SML_BUFF_SIZE];
	uint32_t wlen;
	uint8_t crc;

	wbuf[0] = A1007_CMD_FIRST_BYTE(mem_addr);
	wbuf[1] = (uint8_t)mem_addr;
	wlen = sizeof(mem_addr);

	rlen = len + A1007_DATA_HEAD_SIZE;
	if (unlikely(rlen > A1007_BIG_BUFF_SIZE)) {
		hwlog_err("need too much data in %s for %s\n",
			  __func__, drv_data->name);
		return A1007_FAIL;
	}
	if (a1007_owi_send(drv_data, wlen, wbuf)) {
		hwlog_err("send memory address(%04x) failed for %s\n",
			  mem_addr, drv_data->name);
		return A1007_FAIL;
	}
	if (a1007_owi_recv(drv_data, rlen, rbuf)) {
		hwlog_err("read memory address(%04x) failed for %s\n",
			  mem_addr, drv_data->name);
		return A1007_FAIL;
	}
	if (a1007_get_crc(drv_data, &crc)) {
		hwlog_err("read memory address(%04x) data crc failed for %s\n",
			  mem_addr, drv_data->name);
		return A1007_FAIL;
	}
	if (crc != a1007_crc8_calc(mem_addr, rbuf, rlen)) {
		hwlog_err("crc(%02d) for addr(%04x) data(len:%d) fail for %s\n",
			  crc, mem_addr, len, drv_data->name);
		return A1007_FAIL_CRC;
	}
	memcpy(buf, rbuf + A1007_DATA_HEAD_SIZE, len);

	return A1007_SUCCESS;
}

static int32_t a1007_read_memory(struct a1007_des *drv_data,
				 const uint16_t mem_addr,
				 const uint32_t len, uint8_t *buf)
{
	unsigned char retry;
	int32_t ret;

	for (retry = 0; retry < GET_READ_MEM_RETRY; retry++) {
		ret = __a1007_read_memory(drv_data, mem_addr, len, buf);
		if (ret) {
			hwlog_info("%s failed(%d@%d)", __func__, retry,
				   GET_READ_MEM_RETRY);
			a1007_mix_reset(drv_data, retry);
		} else {
			return A1007_SUCCESS;
		}
	}

	return A1007_FAIL;
}

static int check_ic_names(uint8_t *certz)
{
	if (memcmp(certz, g_org_name, A1007_ORG_NAME_LEN)) {
		hwlog_err("organization name check failed\n");
		return A1007_FAIL;
	}
	if (memcmp(certz + A1007_ORG_NAME_LEN, g_com_name,
	    A1007_COM_NAME_LEN)) {
		hwlog_err("common name check failed\n");
		return A1007_FAIL;
	}

	return A1007_SUCCESS;
}

static int32_t __a1007_get_uid(struct a1007_des *drv_data)
{
	if (a1007_read_memory(drv_data, NXP_CERT_UID_ADDR, A1007_UID_SIZE,
			      A1007_MEM_TO_DATA(drv_data->mem.uid))) {
		hwlog_err("%s failed\n", __func__);
		return A1007_FAIL;
	}

	return A1007_SUCCESS;
}

static int32_t a1007_get_uid(struct a1007_des *drv_data)
{
	if (A1007_MEM_VALID(drv_data->mem.uid))
		return A1007_SUCCESS;
	if (__a1007_get_uid(drv_data))
		return A1007_FAIL;
	A1007_MEM_VALID(drv_data->mem.uid) = 1;

	return A1007_SUCCESS;
}

static int32_t __a1007_get_aut_source(struct a1007_des *drv_data)
{
	uint32_t tsize, rsize;
	uint16_t mem_addr = NXP_CERT_ADDR;
	uint8_t *buf = A1007_MEM_TO_DATA(drv_data->mem.certz);

	for (tsize = 0; tsize < A1007_CERTZ_SIZE; tsize += rsize) {
		if (tsize + A1007_CERTZ_SEG_SIZE > A1007_CERTZ_SIZE)
			rsize = A1007_CERTZ_SIZE - tsize;
		else
			rsize = A1007_CERTZ_SEG_SIZE;
		if (a1007_read_memory(drv_data, mem_addr, rsize, buf)) {
			hwlog_err("%s failed to read addr:%04x data in %s\n",
				  drv_data->name, mem_addr, __func__);
			return A1007_FAIL;
		}
		mem_addr += (uint16_t)rsize;
		buf += rsize;
	}
	if (check_ic_names(A1007_MEM_TO_DATA(drv_data->mem.certz)))
		return A1007_FAIL;

	return A1007_SUCCESS;
}

static int32_t a1007_get_aut_source(struct a1007_des *drv_data)
{
	if (A1007_MEM_VALID(drv_data->mem.certz))
		return A1007_SUCCESS;
	if (__a1007_get_aut_source(drv_data)) {
		hwlog_err("%s failed\n", __func__);
		return A1007_FAIL;
	}
	A1007_MEM_VALID(drv_data->mem.certz) = 1;

	return A1007_SUCCESS;
}

static int32_t a1007_aut_sig(struct a1007_des *drv_data)
{
	uint16_t dev_id;
	uint8_t *uid = NULL;
	uint8_t *certz = NULL;

	/* read device ID */
	if (a1007_get_device_id(drv_data)) {
		hwlog_err("get device id failed in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}
	dev_id = A1007_DEVID_U16(drv_data->mem.dev_id);
	/* read A1007 UID */
	if (a1007_get_uid(drv_data)) {
		hwlog_err("get uid failed in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}
	/* get ic pubkey and certz */
	if (a1007_get_aut_source(drv_data)) {
		hwlog_err("get ecdsa resource failed in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}
	uid = A1007_MEM_TO_DATA(drv_data->mem.uid);
	certz = A1007_MEM_TO_DATA(drv_data->mem.certz);
	if (a1007_verify_certz(g_root_pubkey, dev_id, uid, certz)) {
		hwlog_err("signature is unbelievable for %s\n", drv_data->name);
		A1007_MEM_VALID(drv_data->mem.certz) = 0;
		return A1007_FAIL;
	}

	return A1007_SUCCESS;
}

static int32_t a1007_exe_ecdh(struct a1007_des *drv_data, bitstr_t ecc_rs)
{
	uint8_t sendbuf[A1007_MID_BUFF_SIZE];
	uint32_t len;

	len = a1007_generate_challenge(sendbuf, ecc_rs, drv_data->rand);
	if (a1007_owi_send(drv_data, len, sendbuf)) {
		hwlog_err("send challenge failed in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}

	return A1007_SUCCESS;
}

static int32_t a1007_get_ecdh(struct a1007_des *drv_data, uint8_t *rev,
			      uint8_t *rlen)
{
	uint8_t sendbuf[A1007_SML_BUFF_SIZE];

	sendbuf[0] = A1007_CMD_FIRST_BYTE(A1007_READ_ECDH);
	sendbuf[1] = (uint8_t)A1007_READ_ECDH;

	if (a1007_owi_send(drv_data, A1007_CMD_SIZE, sendbuf)) {
		hwlog_err("get ecdh cmd send failed in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}
	if (a1007_owi_recv(drv_data, A1007_READ_ECDH_LEN, rev)) {
		hwlog_err("get ecdh data failed in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}
	*rlen = A1007_READ_ECDH_LEN;

	return A1007_SUCCESS;
}

static int __a1007_ecdh(struct a1007_des *drv_data)
{
	bitstr_t ecc_rs;
	uint8_t rev[A1007_SML_BUFF_SIZE];
	uint8_t rlen;
	uint8_t *pubkey = NULL;

	if (a1007_get_aut_source(drv_data)) {
		hwlog_err("get aut src failed in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}
	if (a1007_exe_ecdh(drv_data, ecc_rs)) {
		hwlog_err("ask ic to do ecdh failed in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}
	/* wait ic to calculate ecdh */
	msleep(drv_data->wait.ecdh_ms);
	/* now we calculate ecdh */
	pubkey = A1007_MEM_TO_DATA(drv_data->mem.certz) +
	    A1007_CERTZ_PUBKEY_OFF;
	a1007_precompute_secret(ecc_rs, ecc_rs, pubkey);
	/* get ecdh result from ic */
	if (a1007_get_ecdh(drv_data, rev, &rlen)) {
		hwlog_err("get ECDH data fail for %s\n", drv_data->name);
		return A1007_FAIL;
	}
	if (a1007_compute_key_from_response(rev, rlen, ecc_rs,
	    A1007_MEM_TO_DATA(drv_data->mem.mac_context))) {
		hwlog_err("ECDH failed for %s\n", drv_data->name);
		return A1007_FAIL;
	}

	return A1007_SUCCESS;
}

static int _a1007_ecdh(struct a1007_des *drv_data)
{
	unsigned char retry;

	for (retry = 0; retry < GET_ECDH_RETRY; retry++) {
		if (__a1007_ecdh(drv_data)) {
			hwlog_info("%s failed(%d@%d)", __func__, retry,
				   GET_ECDH_RETRY);
			a1007_mix_reset(drv_data, retry);
		} else {
			return A1007_SUCCESS;
		}
	}

	return A1007_FAIL;
}

static int a1007_ecdh(struct a1007_des *drv_data)
{
	if (A1007_MEM_VALID(drv_data->mem.mac_context))
		return A1007_SUCCESS;
	if (_a1007_ecdh(drv_data))
		return A1007_FAIL;
	A1007_MEM_VALID(drv_data->mem.mac_context) = 1;

	return A1007_SUCCESS;
}

static int a1007_ecdh_noretry(struct a1007_des *drv_data)
{
	if (A1007_MEM_VALID(drv_data->mem.mac_context))
		return A1007_SUCCESS;
	if (__a1007_ecdh(drv_data)) {
		hwlog_err("__a1007_ecdh failed in %s for %s", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}
	A1007_MEM_VALID(drv_data->mem.mac_context) = 1;

	return A1007_SUCCESS;
}

static int32_t __a1007_verify_mac(struct a1007_des *drv_data)
{
	uint8_t sendbuf[A1007_SML_BUFF_SIZE];
	uint8_t recvbuf[A1007_SML_BUFF_SIZE];
	uint8_t i = 0;
	uint8_t crc_len;
	uint8_t mac[8] = { 0 };
	uint8_t *mac_source = NULL;
	uint8_t *ic_mac = NULL;
	int32_t ret;

	if (a1007_ecdh(drv_data)) {
		hwlog_err("ecdh failed in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}

	sendbuf[i++] = A1007_CMD_FIRST_BYTE(A1007_EXE_MAC);
	sendbuf[i++] = (uint8_t)A1007_EXE_MAC;
	sendbuf[i++] = 1;
	sendbuf[i++] = A1007_VIRTUAL_PAGE;
	sendbuf[i] = a1007_crc8_calc(A1007_EXE_MAC, sendbuf + A1007_CMD_SIZE,
				     i - A1007_CMD_SIZE);
	if (a1007_owi_send(drv_data, i + 1, sendbuf)) {
		hwlog_err("send exe mac failed in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}
	msleep(drv_data->wait.ecdsa_ms);
	i = 0;
	sendbuf[i++] = A1007_CMD_FIRST_BYTE(A1007_READ_MAC);
	sendbuf[i++] = A1007_READ_MAC & 0x00FF;
	if (a1007_owi_send(drv_data, i, sendbuf)) {
		hwlog_err("send get mac failed in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}
	ret = a1007_owi_recv(drv_data, A1007_READ_MAC_LEN, recvbuf);
	if (ret) {
		hwlog_err("recv mac failed in %s for %s\n", __func__,
			  drv_data->name);
		return ret;
	}
	crc_len = A1007_READ_MAC_LEN - 1;
	if (recvbuf[crc_len] !=
	    a1007_crc8_calc(A1007_READ_MAC, recvbuf, crc_len)) {
		hwlog_err("mac crc failed in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}
	mac_source = recvbuf + A1007_DATA_HEAD_SIZE;
	present80_cbcmac(A1007_MEM_TO_DATA(drv_data->mem.mac_context),
			 mac_source, A1007_MAC_LEN_SIZE, 1, mac);
	present80_cbcmac(A1007_MEM_TO_DATA(drv_data->mem.mac_context),
			 NULL, A1007_BLOCK_SIZE, 0, mac);
	ic_mac = recvbuf + A1007_DATA_HEAD_SIZE + A1007_MAC_LEN_SIZE;
	if (memcmp(mac, ic_mac, A1007_MAC_LEN_SIZE)) {
		hwlog_err("mac mismatch in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}

	return A1007_SUCCESS;
}

static int32_t a1007_verify_mac(struct a1007_des *drv_data)
{
	unsigned char retry;
	int32_t ret;

	for (retry = 0; retry < GET_VERYFY_MAC_RETRY; retry++) {
		ret = __a1007_verify_mac(drv_data);
		if (ret) {
			hwlog_info("%s failed(%d@%d)", __func__, retry,
				   GET_VERYFY_MAC_RETRY);
			a1007_mix_reset(drv_data, retry);
		} else {
			return A1007_SUCCESS;
		}
	}

	return A1007_FAIL;
}

static enum batt_ic_type get_ic_type(void)
{
	return NXP_A1007_TYPE;
}

static int __get_batt_type(struct a1007_des *drv_data)
{
	if (a1007_read_memory(drv_data, USER_CERT_ADDR, BATTERY_TYPE_SIZE,
			      A1007_MEM_TO_DATA(drv_data->mem.btype))) {
		hwlog_err("read memory failed in %s for %s\n", __func__,
			  drv_data->name);
		return A1007_FAIL;
	}
	/* swap device byte order to soft defined order */
	swap(A1007_MEM_TO_DATA(drv_data->mem.btype)[0],
	     A1007_MEM_TO_DATA(drv_data->mem.btype)[1]);

	return A1007_SUCCESS;
}

static int get_batt_type(struct platform_device *pdev,
			 const unsigned char **type, unsigned int *type_len)
{
	struct a1007_des *drv_data = NULL;

	if (!pdev || !type || !type_len)
		return A1007_FAIL;
	drv_data = platform_get_drvdata(pdev);
	if (!drv_data) {
		hwlog_err("Null drv_data found in %s for %s\n",
			  __func__, pdev->name);
		return A1007_FAIL;
	}

	if (A1007_MEM_VALID(drv_data->mem.btype)) {
		*type = A1007_MEM_TO_DATA(drv_data->mem.btype);
		*type_len = BATTERY_TYPE_SIZE;
		return A1007_SUCCESS;
	}
	mutex_lock(&drv_data->ops_mutex);
	if (__get_batt_type(drv_data)) {
		hwlog_err("read memory failed in %s for %s\n",
			  __func__, pdev->name);
		mutex_unlock(&drv_data->ops_mutex);
		return A1007_FAIL;
	}
	*type = A1007_MEM_TO_DATA(drv_data->mem.btype);
	*type_len = BATTERY_TYPE_SIZE;
	A1007_MEM_VALID(drv_data->mem.btype) = 1;
	mutex_unlock(&drv_data->ops_mutex);

	return A1007_SUCCESS;
}

static inline uint8_t is_sn_lock(struct a1007_des *drv_data)
{
	uint8_t *status = A1007_MEM_TO_DATA(drv_data->mem.lock_status);

	return *status & A1007_SN_LOCK_MASK;
}

static int __a1007_eeunlock(struct a1007_des *drv_data, struct batt_res *res)
{
	uint8_t i = 0;
	uint8_t sendbuf[A1007_SML_BUFF_SIZE];

	if (!res || res->len != A1007_MAC_LEN_SIZE) {
		hwlog_err("found illegal res in %s for %s\n",
			  __func__, drv_data->name);
		return A1007_FAIL;
	}

	sendbuf[i++] = A1007_CMD_FIRST_BYTE(A1007_EEUNLOCK);
	sendbuf[i++] = (uint8_t)A1007_EEUNLOCK;
	sendbuf[i++] = res->len;
	memcpy(sendbuf + i, res->data, res->len);
	i += res->len;
	sendbuf[i] = a1007_crc8_calc(A1007_EEUNLOCK, sendbuf + 2, res->len + 1);
	A1007_MEM_VALID(drv_data->mem.lock_status) = 0;
	if (a1007_owi_send(drv_data, i + 1, sendbuf)) {
		hwlog_err("send eeunlock cmd failed in %s for %s\n",
			  __func__, drv_data->name);
		return A1007_FAIL;
	}
	msleep(drv_data->wait.eeunlock_ms);

	return A1007_SUCCESS;
}

static int a1007_eeunlock(struct a1007_des *drv_data, struct batt_res *res)
{
	if (a1007_ecdh_noretry(drv_data)) {
		hwlog_info("a1007_ecdh_noretry failed in %s for %s",
			   __func__, drv_data->name);
		return A1007_FAIL;
	}

	if (a1007_get_lstatus_noretry(drv_data)) {
		hwlog_info("a1007_get_lstatus_noretry failed in %s for %s",
			   __func__, drv_data->name);
		return A1007_FAIL;
	}

	if (!is_sn_lock(drv_data))
		return A1007_SUCCESS;

	if (__a1007_eeunlock(drv_data, res)) {
		hwlog_info("__a1007_eeunlock failed in %s", __func__);
		return A1007_FAIL;
	}

	if (a1007_get_lstatus_noretry(drv_data)) {
		hwlog_info("confirm eeunlock status failed in %s for %s",
			   __func__, drv_data->name);
		return A1007_FAIL;
	}

	if (!is_sn_lock(drv_data))
		return A1007_SUCCESS;

	return A1007_FAIL;
}

static int __a1007_read_sn(struct a1007_des *drv_data, struct batt_res *res)
{
	if (a1007_eeunlock(drv_data, res)) {
		hwlog_err("eeunlock failed in %s for %s\n",
			  __func__, drv_data->name);
		return A1007_FAIL;
	}

	if (__a1007_read_memory(drv_data, NXP_SN_ADDR, drv_data->sn_len,
				A1007_MEM_TO_DATA(drv_data->mem.sn))) {
		hwlog_err("read memory failed in %s for %s\n",
			  __func__, drv_data->name);
		return A1007_FAIL;
	}

	return A1007_SUCCESS;
}

static int _a1007_read_sn(struct a1007_des *drv_data, struct batt_res *res)
{
	unsigned char retry;

	for (retry = 0; retry < GET_SN_RETRY; retry++) {
		if (__a1007_read_sn(drv_data, res)) {
			hwlog_info("%s failed(%d@%d)", __func__, retry,
				   GET_SN_RETRY);
			a1007_mix_reset(drv_data, retry);
		} else {
			return A1007_SUCCESS;
		}
	}
	a1007_hard_reset(drv_data);

	return A1007_FAIL;
}

static int a1007_read_sn(struct a1007_des *drv_data, struct batt_res *res)
{
	if (A1007_MEM_VALID(drv_data->mem.sn))
		return A1007_SUCCESS;
	if (_a1007_read_sn(drv_data, res))
		return A1007_FAIL;
	A1007_MEM_VALID(drv_data->mem.sn) = 1;

	return A1007_SUCCESS;
}

static int prepare(struct platform_device *pdev, enum res_type type,
		   struct batt_res *res)
{
	int ret;
	struct a1007_des *drv_data = NULL;

	if (!pdev)
		return A1007_FAIL;
	drv_data = platform_get_drvdata(pdev);
	if (!drv_data) {
		hwlog_err("drv_data is NULL in %s for %s\n",
			  __func__, pdev->name);
		return A1007_FAIL;
	}

	mutex_lock(&drv_data->ops_mutex);
	switch (type) {
	case RES_SN:
		if (a1007_get_uid(drv_data)) {
			hwlog_err("a1007_get_uid failed in %s for %s",
				  __func__, drv_data->name);
			ret = A1007_FAIL;
			break;
		}
		if (a1007_ecdh(drv_data)) {
			hwlog_err("a1007_ecdh failed in %s for %s",
				  __func__, drv_data->name);
			ret = A1007_FAIL;
			break;
		}
		memcpy(drv_data->mac_src.uid,
		       A1007_MEM_TO_DATA(drv_data->mem.uid), A1007_UID_SIZE);
		memcpy(drv_data->mac_src.mac_context,
		       A1007_MEM_TO_DATA(drv_data->mem.mac_context),
		       sizeof(drv_data->mem.mac_context));
		if (res) {
			res->data = (const unsigned char *)&drv_data->mac_src;
			res->len = sizeof(drv_data->mac_src);
		}
		ret = A1007_SUCCESS;
		break;
	case RES_CT:
		ret = A1007_SUCCESS;
		if (res)
			res->len = 0;
		break;
	default:
		ret = A1007_FAIL;
		break;
	}
	mutex_unlock(&drv_data->ops_mutex);

	return ret;
}

static int get_batt_sn(struct platform_device *pdev, struct batt_res *res,
		       const unsigned char **sn, unsigned int *sn_len_bits)
{
	struct a1007_des *drv_data = NULL;

	if (!pdev || !res)
		return A1007_FAIL;
	drv_data = platform_get_drvdata(pdev);
	if (!drv_data) {
		hwlog_err("Null drv_data found in %s for %s\n",
			  __func__, pdev->name);
		return A1007_FAIL;
	}

	mutex_lock(&drv_data->ops_mutex);
	if (a1007_read_sn(drv_data, res)) {
		hwlog_err("read sn failed in %s for %s\n",
			  __func__, pdev->name);
		goto batt_sn_err;
	}
	if (sn)
		*sn = A1007_MEM_TO_DATA(drv_data->mem.sn);
	if (sn_len_bits)
		*sn_len_bits = drv_data->sn_len;
	mutex_unlock(&drv_data->ops_mutex);

	return A1007_SUCCESS;

batt_sn_err:
	mutex_unlock(&drv_data->ops_mutex);
	return A1007_FAIL;
}

static int __certification(struct platform_device *pdev, struct batt_res *res,
			   enum key_cr *result)
{
	struct a1007_des *drv_data = platform_get_drvdata(pdev);

	if (!drv_data)
		return A1007_FAIL;
	(void)res;
	(void)result;
	if (a1007_aut_sig(drv_data)) {
		hwlog_err("signature is illegal for %s\n", pdev->name);
		goto cert_fail;
	}

	if (a1007_ecdh(drv_data)) {
		hwlog_err("ecdh failed in %s for %s\n", __func__, pdev->name);
		return A1007_FAIL;
	}

	if (a1007_verify_mac(drv_data)) {
		hwlog_err("mac with pubkey unmatch for %s\n", pdev->name);
		goto cert_fail;
	}

	return A1007_SUCCESS;

cert_fail:
	return A1007_FAIL;
}

static int certification(struct platform_device *pdev, struct batt_res *res,
			 enum key_cr *result)
{
	unsigned char retry;
	struct a1007_des *drv_data = NULL;

	if (!pdev || !result)
		return A1007_FAIL;
	drv_data = platform_get_drvdata(pdev);
	mutex_lock(&drv_data->ops_mutex);
	for (retry = 0; retry < VALID_CERTZ_RETRY; retry++) {
		if (__certification(pdev, res, result)) {
			hwlog_info("%s failed(%d@%d)", __func__, retry,
				   VALID_CERTZ_RETRY);
			a1007_mix_reset(drv_data, retry);
		} else {
			mutex_unlock(&drv_data->ops_mutex);
			*result = KEY_PASS;
			return A1007_SUCCESS;
		}
	}
	mutex_unlock(&drv_data->ops_mutex);
	*result = KEY_FAIL_IC;

	return A1007_SUCCESS;
}

static int __a1007_eeperm_writelock(struct a1007_des *drv_data, uint8_t index)
{
	uint8_t wbuf[A1007_SML_BUFF_SIZE];
	uint8_t i = 0;

	wbuf[i++] = A1007_CMD_FIRST_BYTE(A1007_EE_WLOCK);
	wbuf[i++] = (uint8_t)A1007_EE_WLOCK;
	wbuf[i++] = A1007_EE_WLOCK_LEN;
	wbuf[i++] = index;
	wbuf[i] = a1007_crc8_calc(A1007_EE_WLOCK, wbuf + A1007_CMD_SIZE,
				  i - A1007_CMD_SIZE);

	A1007_MEM_VALID(drv_data->mem.lock_status) = 0;
	if (a1007_owi_send(drv_data, i + 1, wbuf)) {
		hwlog_err("write lock eeperm for page(%d) failed\n", index);
		return A1007_FAIL;
	}

	return A1007_SUCCESS;
}

static int _a1007_eeperm_writelock(struct a1007_des *drv_data, uint8_t index)
{
	unsigned char retry;

	for (retry = 0; retry < GET_EEWRT_LOCK_RETRY; retry++) {
		if (__a1007_eeperm_writelock(drv_data, index)) {
			hwlog_info("%s failed(%d@%d)", __func__, retry,
				   GET_EEWRT_LOCK_RETRY);
			a1007_mix_reset(drv_data, retry);
		} else {
			return A1007_SUCCESS;
		}
	}

	return A1007_FAIL;
}

static int set_old_battery(struct a1007_des *drv_data)
{
	return _a1007_eeperm_writelock(drv_data, A1007_SN_BLOCK_IDX);
}

static int __a1007_power_down(struct a1007_des *drv_data)
{
	uint8_t wbuf[A1007_SML_BUFF_SIZE];
	uint32_t i = 0;

	wbuf[i++] = A1007_CMD_FIRST_BYTE(A1007_POWER_DOWN);
	wbuf[i++] = (uint8_t)A1007_POWER_DOWN;
	wbuf[i++] = A1007_PD_PARA_LEN;
	wbuf[i] = A1007_OWI_WAKE;

	if (a1007_owi_send(drv_data, i + 1, wbuf)) {
		hwlog_err("a1007 power down failed\n");
		return A1007_FAIL;
	}
	A1007_MEM_VALID(drv_data->mem.mac_context) = 0;
	drv_data->power_down = 1;
	udelay(drv_data->wait.power_down_us);

	return A1007_SUCCESS;
}

static int a1007_power_down(struct a1007_des *drv_data)
{
	unsigned char retry;

	for (retry = 0; retry < A1007_POWER_DOWN_RETRY; retry++) {
		if (__a1007_power_down(drv_data)) {
			hwlog_info("%s failed(%d@%d)", __func__, retry,
				   A1007_POWER_DOWN_RETRY);
			a1007_hard_reset(drv_data);
		} else {
			return A1007_SUCCESS;
		}
	}

	return A1007_FAIL;
}

static void power_down(struct platform_device *pdev)
{
	struct a1007_des *drv_data = NULL;

	if (!pdev)
		return;
	drv_data = platform_get_drvdata(pdev);
	if (!drv_data)
		return;

	mutex_lock(&drv_data->ops_mutex);
	wake_lock_timeout(&drv_data->wlock, A1007_LOCK_TIMEOUT);
	if (!a1007_power_down(drv_data))
		hwlog_info("%s power down\n", pdev->name);
	else
		hwlog_err("%s power down fail\n", pdev->name);
	wake_unlock(&drv_data->wlock);
	mutex_unlock(&drv_data->ops_mutex);
}

static int set_batt_safe_info(struct platform_device *pdev,
			      enum batt_safe_info_t type, void *value)
{
	struct a1007_des *drv_data = NULL;

	if (!pdev)
		return A1007_FAIL;
	drv_data = platform_get_drvdata(pdev);
	if (!drv_data)
		return A1007_FAIL;
	mutex_lock(&drv_data->ops_mutex);
	wake_lock_timeout(&drv_data->wlock, A1007_LOCK_TIMEOUT);
	switch (type) {
	case BATT_MATCH_ABILITY:
		if (!value)
			break;
		if (*(enum batt_match_type *)value == BATTERY_REMATCHABLE)
			break;
		if (drv_data->power_down) {
			a1007_hard_reset(drv_data);
			drv_data->power_down = 0;
		}
		if (set_old_battery(drv_data))
			hwlog_err("set old fail for %s\n", pdev->name);
		(void)a1007_power_down(drv_data);
	default:
		hwlog_info("unkown safe info type for %s\n", pdev->name);
		break;
	}
	wake_unlock(&drv_data->wlock);
	mutex_unlock(&drv_data->ops_mutex);
	return A1007_SUCCESS;
}

static int get_batt_safe_info(struct platform_device *pdev,
			      enum batt_safe_info_t type, void *value)
{
	struct a1007_des *drv_data = NULL;
	int ret = A1007_FAIL;

	if (!pdev || !value)
		return A1007_FAIL;
	drv_data = platform_get_drvdata(pdev);
	if (!drv_data)
		return A1007_FAIL;

	mutex_lock(&drv_data->ops_mutex);
	wake_lock_timeout(&drv_data->wlock, A1007_LOCK_TIMEOUT);
	switch (type) {
	case BATT_MATCH_ABILITY:
		if (a1007_get_lstatus(drv_data))
			hwlog_err("set old fail for %s in %s\n",
				  pdev->name, __func__);
		if (A1007_MEM_VALID(drv_data->mem.lock_status) &&
		    !is_sn_lock(drv_data))
			*(enum batt_match_type *)value = BATTERY_REMATCHABLE;
		else
			*(enum batt_match_type *)value = BATTERY_UNREMATCHABLE;
		ret = A1007_SUCCESS;
		break;
	default:
		hwlog_err("unsupported safety info type in %s\n", __func__);
		break;
	}
	wake_unlock(&drv_data->wlock);
	mutex_unlock(&drv_data->ops_mutex);

	return ret;
}

#ifdef ONEWIRE_STABILITY_DEBUG
static ssize_t reset_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct a1007_des *drv_data = dev_get_drvdata(dev);

	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "DRIVE DATA IS NULL\n");
	a1007_hard_reset(drv_data);

	return snprintf(buf, PAGE_SIZE, "OK\n");
}

static ssize_t power_down_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct a1007_des *drv_data = dev_get_drvdata(dev);

	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "DRIVE DATA IS NULL\n");
	if (!a1007_power_down(drv_data))
		return snprintf(buf, PAGE_SIZE, "FAIL\n");

	return snprintf(buf, PAGE_SIZE, "OK\n");
}

static ssize_t read_mem_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct a1007_des *drv_data = dev_get_drvdata(dev);
	int i;
	int pos = 0;

	if (!drv_data)
		return snprintf(buf, PAGE_SIZE, "DRIVE DATA IS NULL\n");
	if (!drv_data->buf_len)
		return snprintf(buf, PAGE_SIZE, "\n");
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "mem:%04x len:%d",
			drv_data->mem_addr, drv_data->buf_len);
	for (i = 0; i < drv_data->buf_len; i++) {
		if (i % 16 == 0) {
			snprintf(buf + pos, PAGE_SIZE - pos, "\n0x%04x:",
				 drv_data->mem_addr + i);
		}
		pos += snprintf(buf + pos, PAGE_SIZE - pos, " %02x",
				drv_data->mem_buff[i]);
	}

	return pos;
}

static ssize_t read_mem_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct a1007_des *drv_data = dev_get_drvdata(dev);
	char *sub = NULL, *cur = NULL;
	int temp0, temp1;
	char str[A1007_MID_BUFF_SIZE] = { 0 };
	size_t len;

	if (!drv_data)
		return -1;
	len = min_t(size_t, sizeof(str) - 1, count);
	memcpy(str, buf, len);
	cur = &str[0];
	sub = strsep(&cur, " ");
	if (!sub || kstrtoint(sub, 0, &temp0))
		return -1;
	if (!cur || kstrtoint(cur, 0, &temp1))
		return -1;
	if (temp0 < 0 || temp1 < 0)
		return -1;
	if (((temp0 >= 0x0100 && temp0 < 0x0320) ||
	     (temp0 >= 0x0A00 && temp0 < 0x0D80)) &&
	    (temp1 > 1 && temp1 <= A1007_BLOCK_SIZE)) {
		if (a1007_read_memory(drv_data, temp0, temp1,
				      drv_data->mem_buff)) {
			drv_data->err_cnt++;
			drv_data->mem_addr = 0;
			drv_data->buf_len = snprintf(drv_data->mem_buff,
						     A1007_BLOCK_SIZE,
						     "READ ERROR\n");
		} else {
			drv_data->mem_addr = temp0;
			drv_data->buf_len = temp1;
			return count;
		}
	}

	return -1;
}

static ssize_t crc_test_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct a1007_des *drv_data = dev_get_drvdata(dev);
	uint8_t uid[16];
	int i;
	size_t len = 0;

	if (a1007_read_memory(drv_data, NXP_CERT_UID_ADDR, 16, uid))
		return snprintf(buf + len, PAGE_SIZE, "READ FAIL\n");
	for (i = 0; i < A1007_UID_SIZE; i++)
		len += snprintf(buf + len, PAGE_SIZE, "%02x ", uid[i]);
	len += snprintf(buf + len, PAGE_SIZE, "\n");

	return len;
}

static DEVICE_ATTR_RW(read_mem);
static DEVICE_ATTR_RO(power_down);
static DEVICE_ATTR_RO(reset);
static DEVICE_ATTR_RO(crc_test);
#endif /* ONEWIRE_STABILITY_DEBUG */

static const struct attribute *a1007_attrs[] = {
#ifdef ONEWIRE_STABILITY_DEBUG
	&dev_attr_read_mem.attr,
	&dev_attr_power_down.attr,
	&dev_attr_reset.attr,
	&dev_attr_crc_test.attr,
#endif /* ONEWIRE_STABILITY_DEBUG */
	NULL,			/* sysfs_create_files need last one be NULL */
};

static int a1007_set_up_ops(struct batt_ct_ops *ops)
{
	ops->get_ic_type = get_ic_type;
	ops->get_batt_type = get_batt_type;
	ops->get_batt_sn = get_batt_sn;
	ops->prepare = prepare;
	ops->certification = certification;
	ops->set_batt_safe_info = set_batt_safe_info;
	ops->get_batt_safe_info = get_batt_safe_info;
	ops->power_down = power_down;

	return A1007_SUCCESS;
}

static int a1007_ct_ops_register(struct platform_device *pdev,
				 struct batt_ct_ops *ops)
{
	struct a1007_des *drv_data = platform_get_drvdata(pdev);
	struct list_head *ow_phy_reg_head = get_owi_phy_list_head();
	struct ow_phy_list *pos = NULL;

	if (!drv_data) {
		hwlog_err("%s drv_data NULL in %s\n", pdev->name, __func__);
		return A1007_FAIL;
	}
	if (!ow_phy_reg_head) {
		hwlog_err("no phy-ctrl list useable for %s\n", pdev->name);
		return A1007_FAIL;
	}
	list_for_each_entry(pos, ow_phy_reg_head, node) {
		if (drv_data->phy_ctrl == pos->dev_phandle &&
		    pos->onewire_phy_register_v2 != NULL) {
			if (!pos->onewire_phy_register_v2(&drv_data->ops)) {
				drv_data->gpio = pos->gpio;
				if (!drv_data->gpio) {
					hwlog_err("invalid gpio for %s\n",
						  pdev->name);
					continue;
				}
				if (a1007_get_device_id(drv_data))
					hwlog_err("invalid phy ctrl for %s\n",
						  pdev->name);
				else
					break;
			}
		}
	}
	if (&pos->node == ow_phy_reg_head) {
		hwlog_err("Can't find phy ctrl for %s\n", pdev->name);
		return A1007_FAIL;
	}
	a1007_set_up_ops(ops);

	return A1007_SUCCESS;
}

static void a1007_init_time_req(struct a1007_des *drv_data)
{
	struct ow_treq_v2 *trq = &drv_data->trq;
	struct a1007_cmd_wait *wait = &drv_data->wait;

	/* ic specific time request base on experiments PART I */
	trq->reset_low_ns = 55000;
	trq->reset_us = 2000;
	trq->write_low_ns = 3000;
	trq->write_high_ns = 1000;
	trq->write_bit_ns = 11000;
	trq->read_sample_ns = 1500;
	trq->read_rest_ns = 9500;
	trq->stop_low_ns = 5500;
	trq->stop_high_ns = 5500;
	trq->soft_reset_ns = 7000;

	trq->reset_low_cycs = ns2cycles(trq->reset_low_ns);
	trq->write_low_cycs = ns2cycles(trq->write_low_ns);
	trq->write_high_cycs = ns2cycles(trq->write_high_ns);
	trq->write_bit_cycs = ns2cycles(trq->write_bit_ns);
	trq->read_sample_cycs = ns2cycles(trq->read_sample_ns);
	trq->read_rest_cycs = ns2cycles(trq->read_rest_ns);
	trq->stop_low_cycs = ns2cycles(trq->stop_low_ns);
	trq->stop_high_cycs = ns2cycles(trq->stop_high_ns);
	trq->soft_reset_cycs = ns2cycles(trq->soft_reset_ns);

	/* ic specific time request base on experiments PART II */
	wait->ecdsa_ms = 5;
	wait->ecdh_ms = 50;
	wait->eeprom_write_ms = 4;
	wait->eeunlock_ms = 5;
	wait->power_down_us = 200;

	hwlog_info("reset_low_cycs:%u\n", trq->reset_low_cycs);
	hwlog_info("write_low_cycs:%u\n", trq->write_low_cycs);
	hwlog_info("write_high_cycs:%u\n", trq->write_high_cycs);
	hwlog_info("write_bit_cycs:%u\n", trq->write_bit_cycs);
	hwlog_info("read_sample_cycs:%u\n", trq->read_sample_cycs);
	hwlog_info("read_rest_cycles:%u\n", trq->read_rest_cycs);
	hwlog_info("stop_low_cycs:%u\n", trq->stop_low_cycs);
	hwlog_info("stop_high_cycs:%u\n", trq->stop_high_cycs);
	hwlog_info("soft_reset_cycs:%u\n", trq->soft_reset_cycs);
}

static struct a1007_des *a1007_init_drive_data(struct platform_device *pdev)
{
	struct a1007_des *drv_data = devm_kzalloc(&pdev->dev, sizeof(*drv_data),
						  GFP_KERNEL);

	if (!drv_data) {
		hwlog_err("alloc for %s's drv_data failed\n", pdev->name);
		goto a1007_data_init_fail;
	}

	if (of_property_read_u8(pdev->dev.of_node, "addr", &drv_data->addr)) {
		hwlog_err("get addr from dts failed for %s\n", pdev->name);
		goto a1007_data_init_fail;
	}
	if (of_property_read_u8(pdev->dev.of_node, "sn-len",
	    &drv_data->sn_len)) {
		hwlog_err("get SN length failed for %s\n", pdev->name);
		goto a1007_data_init_fail;
	}
	if (of_property_read_u32(pdev->dev.of_node, "phy-ctrl",
				 &drv_data->phy_ctrl)) {
		hwlog_err("get phy-ctrl phandle failed for %s\n", pdev->name);
		goto a1007_data_init_fail;
	}
	if (sysfs_create_files(&pdev->dev.kobj, a1007_attrs))
		hwlog_info("create sysfs files failed for %s\n", pdev->name);
	platform_set_drvdata(pdev, drv_data);
	drv_data->addr = A1007_SLAVE_ADDR;
	drv_data->name = pdev->name;
	get_random_bytes(drv_data->rand, sizeof(drv_data->rand));
	mutex_init(&drv_data->ops_mutex);
	wake_lock_init(&drv_data->wlock, WAKE_LOCK_SUSPEND, pdev->name);

	return drv_data;

a1007_data_init_fail:
	return NULL;
}

static int a1007_driver_probe(struct platform_device *pdev)
{
	struct a1007_des *drv_data = NULL;

	if (!pdev)
		return A1007_FAIL;
	hwlog_info("a1007: %s probing\n", pdev->name);
	/* record ds28el15 platform_device */
	drv_data = a1007_init_drive_data(pdev);
	if (!drv_data)
		return A1007_FAIL;

	a1007_init_time_req(drv_data);

	/* add device to struct batt_ct_ops_list */
	drv_data->reg_node.ic_memory_release = NULL;
	drv_data->reg_node.ct_ops_register = a1007_ct_ops_register;
	drv_data->reg_node.ic_dev = pdev;
	add_to_aut_ic_list(&drv_data->reg_node);

	return A1007_SUCCESS;
}

static int a1007_driver_remove(struct platform_device *pdev)
{
	return A1007_SUCCESS;
}

static const struct of_device_id A1007_match_table[] = {
	{ .compatible = "NXP,A1007,ECC", },
	{ /* end */ },
};

static struct platform_driver a1007_driver = {
	.probe = a1007_driver_probe,
	.remove = a1007_driver_remove,
	.driver = {
		.name = "NXP_A1007_OW",
		.owner = THIS_MODULE,
		.of_match_table = A1007_match_table,
	},
};

int __init a1007_driver_init(void)
{
	return platform_driver_register(&a1007_driver);
}

void __exit a1007_driver_exit(void)
{
	platform_driver_unregister(&a1007_driver);
}

subsys_initcall_sync(a1007_driver_init);
module_exit(a1007_driver_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("a1007 ic");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
