/*
 * ext_sensorhub_debug.c
 *
 * driver for debug with external sensorhub
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

#include "ext_sensorhub_debug.h"

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/slab.h>
#include "securec.h"
#include "ext_sensorhub_route.h"
#include "ext_sensorhub_frame.h"

#define HWLOG_TAG ext_sensorhub_dbg
HWLOG_REGIST();
#define SYS_CLS_MODE 0660
#define DBG_INFO_LINE_SIZE 1024

static const unsigned int char_num_of_byte = 2;
static const unsigned int bit_num_of_hex = 4;
static const char zero_char = '0';
static const unsigned int decimal = 10;
static const char a_char = 'a';
static const unsigned int hex_header_len = 2;
static const unsigned int even_num_factor = 2;

static inline bool is_space_ch(char ch)
{
	return (ch == ' ') || (ch == '\t');
}

static bool end_of_string(char ch)
{
	bool ret = false;

	switch (ch) {
	case '\0':
	case '\r':
	case '\n':
		ret = true;
		break;
	default:
		ret = false;
		break;
	}

	return ret;
}

/* find first pos */
static const char *get_str_begin(const char *cmd_buf)
{
	if (!cmd_buf)
		return NULL;

	while (is_space_ch(*cmd_buf))
		++cmd_buf;

	if (end_of_string(*cmd_buf))
		return NULL;

	return cmd_buf;
}

/* find last pos */
static const char *get_str_end(const char *cmd_buf)
{
	if (!cmd_buf)
		return NULL;

	while (!is_space_ch(*cmd_buf) && !end_of_string(*cmd_buf))
		++cmd_buf;

	return cmd_buf;
}

static bool str_fuzzy_match(const char *cmd_buf, const char *target)
{
	if (!cmd_buf || !target)
		return false;

	for (; !is_space_ch(*cmd_buf) && !end_of_string(*cmd_buf) && *target;
		++target) {
		if (*cmd_buf == *target)
			++cmd_buf;
	}

	return is_space_ch(*cmd_buf) || end_of_string(*cmd_buf);
}

static const struct write_cmd_tag_map tag_map_tab[] = {
	{ "sendcmd", tag_sendcmd },
	{ "subcmds", tag_subcmd },
	{ "mockdata", tag_mock_data },
};

static int get_write_tag(const char *str)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(tag_map_tab); ++i) {
		if (str_fuzzy_match(str, tag_map_tab[i].str))
			return tag_map_tab[i].tag;
	}
	return -EINVAL;
}

static int make_hex_value(unsigned char *val, const char *str)
{
	for (; !is_space_ch(*str) && !end_of_string(*str); ++str) {
		if (!ch_is_hex_digit(*str))
			return -EINVAL;
		*val = (*val) << bit_num_of_hex;
		*val = (*val) | (ch_is_digit(*str) ?
			(*str - '0') : (((*str | 0x20) - a_char) + decimal));
	}

	return 0;
}

static int make_value(unsigned char *val, const char *str)
{
	for (; !is_space_ch(*str) && !end_of_string(*str); ++str) {
		if (!ch_is_digit(*str))
			return -EINVAL;
		*val = (*val) * decimal;
		*val = (*val) + (*str - '0');
	}

	return 0;
}

static int sub_cmds(const char *str)
{
	int index = 0;
	unsigned char *sids = NULL;
	unsigned char *cids = NULL;
	int sid_cnt = 0;
	int cid_cnt = 0;
	unsigned char val;
	int ret;

	/* kfree in ext sensorhub route after used */
	sids = kmalloc(MAX_CMD_BUF_ARGC, GFP_KERNEL);
	cids = kmalloc(MAX_CMD_BUF_ARGC, GFP_KERNEL);
	if (!sids || !cids)
		return -ENOMEM;

	for (; (str = get_str_begin(str));
	str = get_str_end(str)) {
		val = 0;
		if ((*str == '0') && ((*(str + 1) == 'x') ||
				      (*(str + 1) == 'X'))) {
			str += hex_header_len;
			if (make_hex_value(&val, str) < 0)
				return -EINVAL;
		} else {
			if (make_value(&val, str) < 0)
				return -EINVAL;
		}
		if (sid_cnt >= MAX_CMD_BUF_ARGC || cid_cnt >= MAX_CMD_BUF_ARGC)
			break;
		if (index % even_num_factor == 0)
			sids[sid_cnt++] = val;
		else
			cids[cid_cnt++] = val;
		index++;
	}
	if (sid_cnt != cid_cnt) {
		hwlog_err("%s sid and cid count not match", __func__);
		return -EINVAL;
	}

	ret = ext_sensorhub_route_reg_cb(ROUTE_DEBUG_PORT, sids, cids,
					 cid_cnt, NULL);

	hwlog_info("%s subscribe cmd ret: %d", __func__, ret);
	return ret;
}

static int make_frame_data(const char *str, unsigned char *send_buffer, int cnt)
{
	unsigned char val = 0;
	int buffer_cnt = 0;
	int index = 0;

	/* only get first param */
	str = get_str_begin(str);
	for (; !is_space_ch(*str) && !end_of_string(*str); ++str) {
		if (!ch_is_hex_digit(*str))
			return -EINVAL;

		val <<= bit_num_of_hex;
		val |= (ch_is_digit(*str) ?
			(*str - zero_char) :
			(((*str | 0x20) - a_char) + decimal));
		if (buffer_cnt >= cnt) {
			hwlog_err("%s Cannot get more bytes", __func__);
			return -EINVAL;
		}
		if (index % even_num_factor == 1) {
			/* get one byte */
			send_buffer[buffer_cnt++] = val;
			val = 0;
		}
		index++;
	}

	return buffer_cnt;
}

static int handle_frame_cmd(const char *str, int cmd_tag)
{
	/* send data with header without crc */
	/* eg: 215A000e00150B8109020162030100040100 */
	/* each two char in string make one byte */
	unsigned char *send_buffer = NULL;
	int buffer_cnt = 0;
	int ret;

	/* strlen(str) contains last char '0' */
	if (strlen(str) % char_num_of_byte != 1) {
		hwlog_err("%s Invalid parameter length, len:%d",
			  __func__, strlen(str));
		return -EINVAL;
	}

	send_buffer = kmalloc(MAX_FRAME_BUF_LEN, GFP_KERNEL);
	if (!send_buffer)
		return -ENOMEM;

	memset_s(send_buffer, MAX_FRAME_BUF_LEN, 0, MAX_FRAME_BUF_LEN);
	/* only get first param */
	buffer_cnt = make_frame_data(str, send_buffer, MAX_FRAME_BUF_LEN);
	if (buffer_cnt <= 0) {
		kfree(send_buffer);
		send_buffer = NULL;
		return buffer_cnt;
	}

	if (cmd_tag == tag_sendcmd)
		ret = send_debug_frame(send_buffer, buffer_cnt);
	else
		ret = mock_debug_frame(send_buffer, buffer_cnt);
	if (ret < 0)
		hwlog_err("%s send cmd error ret: %d", __func__, ret);
	else
		hwlog_info("%s send cmd success", __func__);

	kfree(send_buffer);
	send_buffer = NULL;
	return ret;
}

static int send_cmd(const char *str)
{
	return handle_frame_cmd(str, tag_sendcmd);
}

static int mock_data(const char *str)
{
	return handle_frame_cmd(str, tag_mock_data);
}

static int parse_str(const char *cmd_buf)
{
	int tag = -1;

	for (; (cmd_buf = get_str_begin(cmd_buf));
		cmd_buf = get_str_end(cmd_buf)) {
		if (tag == -1) {
			tag = get_write_tag(cmd_buf);
			hwlog_info("get_write_tag, %d", tag);
		} else {
			/* do not move this break */
			break;
		}
	}
	switch (tag) {
	case tag_subcmd:
		return sub_cmds(cmd_buf);
	case tag_sendcmd:
		return send_cmd(cmd_buf);
	case tag_mock_data:
		return mock_data(cmd_buf);
	default:
		return -EINVAL;
	}
}

static ssize_t ext_sensorhub_dbg_write(struct class *cls,
				       struct class_attribute *attr,
				       const char *buf, size_t size)
{
	parse_str(buf);
	return size;
}

static int make_show_data(char *buf, unsigned char service_id,
			  unsigned char command_id, unsigned char *read_buffer,
			  ssize_t len)
{
	int ret;
	int i;

	ret = snprintf_s(buf + strlen(buf), DBG_INFO_LINE_SIZE,
			 DBG_INFO_LINE_SIZE - strlen(buf) - 1,
			 "sid: 0x%02x ,cid: 0x%02x ,data len: %d data: ",
			 service_id, command_id, len);
	if (ret < 0 || ret > DBG_INFO_LINE_SIZE)
		return ret;
	/* max data count to show */
	for (i = 0; i < len; ++i) {
		ret = snprintf_s(buf + strlen(buf),
				 DBG_INFO_LINE_SIZE - strlen(buf),
				 DBG_INFO_LINE_SIZE - strlen(buf) - 1,
				 "0x%02x ", read_buffer[i]);
		if (ret < 0 || ret > DBG_INFO_LINE_SIZE)
			return ret;
	}
	ret = snprintf_s(buf + strlen(buf),
			 DBG_INFO_LINE_SIZE - strlen(buf),
			 DBG_INFO_LINE_SIZE - strlen(buf) - 1, "\n");
	if (ret < 0 || ret > DBG_INFO_LINE_SIZE)
		return ret;

	return 0;
}

static ssize_t ext_sensorhub_dbg_read(struct class *cls,
				      struct class_attribute *attr, char *buf)
{
	unsigned char *read_buffer = NULL;
	unsigned char service_id;
	unsigned char command_id;
	int ret;
	ssize_t len;

	read_buffer = kmalloc(MAX_FRAME_BUF_LEN, GFP_KERNEL);
	if (!read_buffer)
		return -ENOMEM;

	memset_s(read_buffer, MAX_FRAME_BUF_LEN, 0, MAX_FRAME_BUF_LEN);
	len = ext_sensorhub_route_read_kernel(ROUTE_DEBUG_PORT, read_buffer,
					      MAX_FRAME_BUF_LEN,
					      &service_id, &command_id);
	if (len < 0) {
		ret = snprintf_s(buf, DBG_INFO_LINE_SIZE,
				 DBG_INFO_LINE_SIZE - 1,
				 "read one package error\n");
		if (ret < 0 || ret > DBG_INFO_LINE_SIZE)
			hwlog_err("append data error");
		goto err;
	}
	ret = make_show_data(buf, service_id, command_id, read_buffer, len);
	if (ret != 0)
		hwlog_err("append show data error, ret: %d", ret);
err:
	kfree(read_buffer);
	read_buffer = NULL;
	return strlen(buf);
}

static ssize_t ext_sensorhub_state_write(struct class *cls,
					 struct class_attribute *attr,
					 const char *buf, size_t size)
{
	/* do nothing */
	return size;
}

static unsigned char *check_state(struct commu_state *state)
{
	int i;

	for (i = 0; i < state->commu_cnt; ++i) {
		if (!state->handshake[i] || state->test_sync_ret[i] <= 0)
			return "failed";
	}

	return "succeeded";
}

static unsigned char *check_commu_status(struct commu_state *state, int index)
{
	if (state->handshake[index] && state->test_sync_ret[index] > 0)
		return "succeeded";

	if (state->handshake[index])
		return "handshaked";

	return "failed";
}

static ssize_t ext_sensorhub_state_read(struct class *cls,
					struct class_attribute *attr, char *buf)
{
	struct commu_state *state;
	int ret;
	int i;

	state = get_commu_state();
	if (!state) {
		ret = snprintf_s(buf, DBG_INFO_LINE_SIZE,
				 DBG_INFO_LINE_SIZE - 1, "get no state\n");
		if (ret < 0 || ret > DBG_INFO_LINE_SIZE)
			hwlog_err("%s, append data error", __func__);
		goto err;
	}

	ret = snprintf_s(buf + strlen(buf), DBG_INFO_LINE_SIZE - strlen(buf),
			 DBG_INFO_LINE_SIZE - strlen(buf) - 1,
			 "\n\t Overview: [%s]\n\t Details:\n",
			 check_state(state));
	if (ret < 0 || ret > DBG_INFO_LINE_SIZE) {
		hwlog_err("%s, append data error", __func__);
		goto err;
	}
	for (i = 0; i < state->commu_cnt; ++i) {
		ret = snprintf_s(buf + strlen(buf),
				 DBG_INFO_LINE_SIZE - strlen(buf),
				 DBG_INFO_LINE_SIZE - strlen(buf) - 1,
				 "\t\t *** [%s]: [%s]. handshake:%d, sync test ret:%d ***\n",
				 state->name[i],
				 check_commu_status(state, i),
				 state->handshake[i],
				 state->test_sync_ret[i]);
		if (ret < 0 || ret > DBG_INFO_LINE_SIZE)
			hwlog_err("%s, append data error", __func__);
	}
	ret = snprintf_s(buf + strlen(buf), DBG_INFO_LINE_SIZE - strlen(buf),
			 DBG_INFO_LINE_SIZE - strlen(buf) - 1, "\n");
	if (ret < 0 || ret > DBG_INFO_LINE_SIZE)
		hwlog_err("%s, append data error", __func__);
err:
	return strlen(buf);
}

static struct class_attribute class_attr_sensorhub_dbg =
	__ATTR(ext_sensorhub_dbg, SYS_CLS_MODE, ext_sensorhub_dbg_read,
	       ext_sensorhub_dbg_write);

static struct class_attribute class_attr_sensorhub_state =
	__ATTR(ext_sensorhub_state, SYS_CLS_MODE, ext_sensorhub_state_read,
	       ext_sensorhub_state_write);

static struct class *ext_sensorhub_dbg_class;
static int ext_sensorhub_dbg_init(void)
{
	if (is_ext_sensorhub_disabled())
		return -EINVAL;

	ext_sensorhub_dbg_class = class_create(THIS_MODULE, "ext_sensors");
	if (IS_ERR(ext_sensorhub_dbg_class))
		return PTR_ERR(ext_sensorhub_dbg_class);

	if (class_create_file(ext_sensorhub_dbg_class,
			      &class_attr_sensorhub_dbg))
		hwlog_warn("%s create files failed in %s", __func__);

	if (class_create_file(ext_sensorhub_dbg_class,
			      &class_attr_sensorhub_state))
		hwlog_warn("%s create files failed in %s", __func__);
	return 0;
}

static void ext_sensorhub_dbg_exit(void)
{
	if (is_ext_sensorhub_disabled())
		return;

	class_destroy(ext_sensorhub_dbg_class);
}

late_initcall_sync(ext_sensorhub_dbg_init);
module_exit(ext_sensorhub_dbg_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("External Sensorhub debug driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
