/*
 * lcd_kit_parse.c
 *
 * lcdkit parse function for lcdkit
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

#include "lcd_kit_common.h"
#include "lcd_kit_dbg.h"
#include "lcd_kit_parse.h"

int conver2le(uint32_t *src, uint32_t *desc, int len)
{
	int i;
	uint32_t temp;

	if ((src == NULL) || (desc == NULL)) {
		LCD_KIT_ERR("src or desc is null\n");
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < len; i++) {
		temp = ((src[i] & 0xff) << 24);
		temp |= (((src[i] >> 8) & 0xff) << 16);
		temp |= (((src[i] >> 16) & 0xff) << 8);
		temp |= ((src[i] >> 24) & 0xff);
		desc[i] = temp;
		temp = 0;
	}
	return LCD_KIT_OK;
}

int lcd_kit_parse_array_data(struct device_node *np, const char *name,
	struct lcd_kit_array_data *out)
{
	int blen = 0;
	uint32_t *data = NULL;
	uint32_t *buf = NULL;

#if defined(LCD_KIT_DEBUG_ENABLE) && \
	(defined(CONFIG_LCD_KIT_HISI) || defined(CONFIG_LCD_KIT_MTK))
	if (is_dpd_mode()) {
		lcd_parse_array(np, name, out);
		return LCD_KIT_OK;
	}
#endif
	data = (uint32_t *)of_get_property(np, name, &blen);
	if (!data) {
		LCD_KIT_ERR("get data fail\n");
		return LCD_KIT_FAIL;
	}
	blen = blen / 4; // 4 means u32 divide u8
	buf = kzalloc(blen * sizeof(uint32_t), GFP_KERNEL);
	if (!buf) {
		LCD_KIT_ERR("alloc buf fail\n");
		return LCD_KIT_FAIL;
	}
	memcpy(buf, data, blen * sizeof(uint32_t));
	conver2le(buf, buf, blen);
	out->buf = buf;
	out->cnt = blen;
	return LCD_KIT_OK;
}

int lcd_kit_parse_u32_array(const struct device_node *np, const char *name,
	uint32_t out[], int len)
{
	int blen = 0;
	uint32_t *data = NULL;

	if (!np || !name || !out) {
		LCD_KIT_ERR("pointer is null\n");
		return LCD_KIT_FAIL;
	}
	data = (uint32_t *)of_get_property(np, name, &blen);
	if (!data) {
		LCD_KIT_ERR("get data fail\n");
		return LCD_KIT_FAIL;
	}
	blen = blen / 4; // 4 means u32 divide u8
	if (len != blen) {
		LCD_KIT_ERR("blen not equal len\n");
		return LCD_KIT_FAIL;
	}
	memcpy(out, data, blen * sizeof(uint32_t));
	(void)conver2le(out, out, blen);
	return LCD_KIT_OK;
}

int lcd_kit_parse_arrays_data(struct device_node *np, const char *name,
	struct lcd_kit_arrays_data *out, int num)
{
	int i, len;
	int cnt = 0;
	int blen = 0;
	uint32_t *data = NULL;
	uint32_t *buf = NULL;
	uint32_t *bp = NULL;

#if defined(LCD_KIT_DEBUG_ENABLE) && \
	(defined(CONFIG_LCD_KIT_HISI) || defined(CONFIG_LCD_KIT_MTK))
	if (is_dpd_mode()) {
		lcd_parse_arrays(np, name, out, num);
		return LCD_KIT_OK;
	}
#endif
	data = (uint32_t *)of_get_property(np, name, &blen);
	if (!data) {
		LCD_KIT_ERR("parse property %s error\n", name);
		return LCD_KIT_FAIL;
	}
	len = blen / 4; // 4 means u32 divide u8
	buf = kzalloc(sizeof(uint32_t) * len, GFP_KERNEL);
	if (!buf) {
		LCD_KIT_ERR("alloc buf fail\n");
		return LCD_KIT_FAIL;
	}
	memcpy(buf, data, len * sizeof(uint32_t));
	conver2le(buf, buf, len);
	while (len >= num) {
		if (num > len) {
			LCD_KIT_ERR("data length = %x error\n", len);
			goto exit_free;
		}
		len -= num;
		cnt++;
	}
	if (len != 0) {
		LCD_KIT_ERR("dts data parse error! data len = %d\n", len);
		goto exit_free;
	}
	out->cnt = cnt;
	out->arry_data = kzalloc(sizeof(struct lcd_kit_array_data) * cnt,
		GFP_KERNEL);
	if (!out->arry_data) {
		LCD_KIT_ERR("kzalloc fail\n");
		goto exit_free;
	}
	bp = buf;
	for (i = 0; i < cnt; i++) {
		out->arry_data[i].cnt = num;
		out->arry_data[i].buf = bp;
		bp += num;
	}
	return LCD_KIT_OK;
exit_free:
	kfree(buf);
	return LCD_KIT_FAIL;
}

static int lcd_scan_dcs_cmd(char *buf, const int *buflen,
	struct lcd_kit_dsi_panel_cmds *pcmds,
	struct lcd_kit_dsi_cmd_desc_header *dchdr)
{
	int i, len;
	char *bp = NULL;
	int cnt = 0;

	bp = buf;
	len = *buflen;
	while (len >= (int)sizeof(struct lcd_kit_dsi_cmd_desc_header)) {
		dchdr = (struct lcd_kit_dsi_cmd_desc_header *)bp;
		bp += sizeof(struct lcd_kit_dsi_cmd_desc_header);
		len -= (int)sizeof(struct lcd_kit_dsi_cmd_desc_header);
		if (dchdr->dlen > len) {
			LCD_KIT_ERR("cmd = 0x%x parse error, len = %d\n",
				dchdr->dtype, dchdr->dlen);
			return LCD_KIT_FAIL;
		}
		bp += dchdr->dlen;
		len -= dchdr->dlen;
		cnt++;
	}
	if (len != 0) {
		LCD_KIT_ERR("dcs_cmd parse error! cmd len = %d\n", len);
		return LCD_KIT_FAIL;
	}
	pcmds->cmds = kzalloc(cnt * sizeof(struct lcd_kit_dsi_cmd_desc),
		GFP_KERNEL);
	if (!pcmds->cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	pcmds->cmd_cnt = cnt;
	pcmds->buf = buf;
	pcmds->blen = *buflen;
	bp = buf;
	len = *buflen;
	for (i = 0; i < cnt; i++) {
		dchdr = (struct lcd_kit_dsi_cmd_desc_header *)bp;
		len -= (int)sizeof(struct lcd_kit_dsi_cmd_desc_header);
		bp += sizeof(struct lcd_kit_dsi_cmd_desc_header);
		pcmds->cmds[i].dtype    = dchdr->dtype;
		pcmds->cmds[i].last     = dchdr->last;
		pcmds->cmds[i].vc       = dchdr->vc;
		pcmds->cmds[i].ack      = dchdr->ack;
		pcmds->cmds[i].wait     = dchdr->wait;
		pcmds->cmds[i].waittype = dchdr->waittype;
		pcmds->cmds[i].dlen     = dchdr->dlen;
		pcmds->cmds[i].payload  = bp;
		bp += dchdr->dlen;
		len -= dchdr->dlen;
	}
	pcmds->flags = LCD_KIT_CMD_REQ_COMMIT;
	return LCD_KIT_OK;
}

int lcd_kit_parse_dcs_cmds(struct device_node *np, char *cmd_key,
	const char *link_key, struct lcd_kit_dsi_panel_cmds *pcmds)
{
	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	const char *data = NULL;
	int blen = 0;
	int buflen = 0;
	int ret = LCD_KIT_OK;
	char *buf = NULL;
	struct lcd_kit_dsi_cmd_desc_header *dchdr = NULL;

	if (!np || !cmd_key || !link_key || !pcmds) {
		LCD_KIT_ERR("pointer is null\n");
		return LCD_KIT_FAIL;
	}
#if defined(LCD_KIT_DEBUG_ENABLE) && \
	(defined(CONFIG_LCD_KIT_HISI) || defined(CONFIG_LCD_KIT_MTK))
	if (is_dpd_mode()) {
		lcd_parse_dcs(np, cmd_key, link_key, pcmds);
		return LCD_KIT_OK;
	}
#endif
	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("adapt_ops is null!\n");
		return LCD_KIT_FAIL;
	}
	memset(pcmds, 0, sizeof(struct lcd_kit_dsi_panel_cmds));
	data = (char *)of_get_property(np, cmd_key, &blen);
	if (!data || (blen == 0)) {
		LCD_KIT_ERR(" failed, key = %s\n", cmd_key);
		return LCD_KIT_FAIL;
	}
	if (adapt_ops->buf_trans) {
		ret = adapt_ops->buf_trans(data, blen, &buf, &buflen);
		if (ret) {
			LCD_KIT_ERR("buf_trans fail\n");
			return LCD_KIT_FAIL;
		}
	}
	if (ret) {
		LCD_KIT_ERR("buffer trans fail!\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("buf is null!\n");
		return LCD_KIT_FAIL;
	}
	/* scan dcs commands */
	ret = lcd_scan_dcs_cmd(buf, &buflen, pcmds, dchdr);
	if (ret < 0) {
		kfree(buf);
		return LCD_KIT_FAIL;
	}
	/* Set default link state to HS Mode */
	pcmds->link_state = LCD_KIT_DSI_HS_MODE;
	if (link_key) {
		data = (char *)of_get_property(np, link_key, NULL);
		if (data && !strcmp(data, "dsi_lp_mode"))
			pcmds->link_state = LCD_KIT_DSI_LP_MODE;
		else
			pcmds->link_state = LCD_KIT_DSI_HS_MODE;
	}
	return LCD_KIT_OK;
}

int lcd_kit_parse_dirty_info(const struct device_node *np, const char *prop_name,
	int *out)
{
	unsigned int value = 0xffff; // init to 0xffff, represent - 1 with 0xffff

	if (!np || !out || !prop_name) {
		LCD_KIT_ERR("pointer is null\n");
		return LCD_KIT_FAIL;
	}
#if defined(LCD_KIT_DEBUG_ENABLE) && \
	(defined(CONFIG_LCD_KIT_HISI) || defined(CONFIG_LCD_KIT_MTK))
	if (is_dpd_mode()) {
		lcd_parse_dirty_region(np, prop_name, out);
		return LCD_KIT_OK;
	}
#endif
	if (of_property_read_u32(np, prop_name, &value)) {
		LCD_KIT_INFO("of_property_read_u32: %s not find\n", prop_name);
		*out = -1; // read device-tree fail, use default value:-1
	}
	/* device-tree can not save -1, use 0xffff to stand for -1 */
	if (value >= 0xffff)
		*out = -1;
	else
		*out = (int)value;
	return LCD_KIT_OK;
}

int lcd_kit_parse_u64(const struct device_node *np, const char *prop_name,
	uint64_t *out, uint64_t def)
{
	uint64_t temp = 0;

	if (!np || !out || !prop_name) {
		LCD_KIT_ERR("pointer is null\n");
		return LCD_KIT_FAIL;
	}
	if (of_property_read_u64(np, prop_name, &temp)) {
		LCD_KIT_INFO("of_property_read: %s not find\n", prop_name);
		*out = def;
		return LCD_KIT_OK;
	}
	*out = temp;
	return LCD_KIT_OK;
}

int lcd_kit_parse_u32(const struct device_node *np, const char *prop_name,
	unsigned int *out, unsigned int def)
{
	unsigned int temp = 0;

	if (!np || !out || !prop_name) {
		LCD_KIT_ERR("pointer is null\n");
		return LCD_KIT_FAIL;
	}
#if defined(LCD_KIT_DEBUG_ENABLE) && \
	(defined(CONFIG_LCD_KIT_HISI) || defined(CONFIG_LCD_KIT_MTK))
	if (is_dpd_mode()) {
		lcd_parse_u32(np, prop_name, out, def);
		return LCD_KIT_OK;
	}
#endif
	if (of_property_read_u32(np, prop_name, &temp)) {
		LCD_KIT_INFO("of_property_read: %s not find\n", prop_name);
		*out = def;
		return LCD_KIT_OK;
	}
	*out = temp;
	return LCD_KIT_OK;
}

int lcd_kit_parse_u8(const struct device_node *np, const char *prop_name,
	unsigned char *out, unsigned char def)
{
	unsigned int temp = 0;

	if (!np || !out || !prop_name) {
		LCD_KIT_ERR("pointer is null\n");
		return LCD_KIT_FAIL;
	}
#if defined(LCD_KIT_DEBUG_ENABLE) && \
	(defined(CONFIG_LCD_KIT_HISI) || defined(CONFIG_LCD_KIT_MTK))
	if (is_dpd_mode()) {
		lcd_parse_u8(np, prop_name, out, def);
		return LCD_KIT_OK;
	}
#endif
	if (of_property_read_u32(np, prop_name, &temp)) {
		LCD_KIT_INFO("of_property_read: %s not find\n", prop_name);
		*out = def;
		return LCD_KIT_OK;
	}
	*out = (unsigned char)temp;
	return LCD_KIT_OK;
}

int lcd_kit_parse_string_array(const struct device_node *np, const char *prop_name,
	const char **out_strs, size_t sz)
{
	if (!np || !prop_name || !out_strs) {
		LCD_KIT_ERR("pointer is null\n");
		return LCD_KIT_FAIL;
	}
#if defined(LCD_KIT_DEBUG_ENABLE) && \
	(defined(CONFIG_LCD_KIT_HISI) || defined(CONFIG_LCD_KIT_MTK))
	if (is_dpd_mode()) {
		lcd_parse_string_array(np, prop_name, out_strs);
		return LCD_KIT_OK;
	}
#endif
	return of_property_read_string_array(np, prop_name, out_strs, sz);
}
int lcd_kit_parse_string(const struct device_node *np, const char *prop_name,
	const char **out_strs)
{
	if (!np || !prop_name || !out_strs) {
		LCD_KIT_ERR("pointer is null\n");
		return LCD_KIT_FAIL;
	}
#if defined(LCD_KIT_DEBUG_ENABLE) && \
	(defined(CONFIG_LCD_KIT_HISI) || defined(CONFIG_LCD_KIT_MTK))
	if (is_dpd_mode()) {
		lcd_parse_string_array(np, prop_name, out_strs);
		return LCD_KIT_OK;
	}
#endif
	return of_property_read_string(np, prop_name, out_strs);
}
