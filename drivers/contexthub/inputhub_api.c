/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub IPC send api.
 * Author: Huawei
 * Create: 2017-06-08
 */

#include <securec.h>
#include "inputhub_api.h"


/*lint -e655*/
/*
 * 发送IPC消息给contexthub并等待回复
 */
int send_cmd_format(unsigned char cmd_tag, unsigned char cmd_type,
    unsigned int subtype, const char *buf, size_t count, bool is_lock, struct read_info *rd)
{
	char auto_buffer[MAX_PKT_LENGTH] = {0};
	char *buffer = NULL;
	int ret = 0;

	struct write_info winfo;
	(void)memset_s(&winfo, sizeof(winfo), 0, sizeof(winfo));
	winfo.tag = cmd_tag;
	winfo.cmd = cmd_type;
	if (rd != NULL) {
		buffer = rd->data;
	} else {
		buffer = auto_buffer;
	}
	pr_info("[%s] : cmd_tag[0x%x] cmd[%d:0x%x]  count[%ld]\n",
		__func__, cmd_tag, cmd_type, subtype, count);

	if (CMD_CMN_CONFIG_REQ != cmd_type) {
		if (count > (MAX_PKT_LENGTH - sizeof(struct pkt_header))) {
			pr_err("%s cnt error[%ld]\n", __func__, count);
			return -ENOMEM;
		}

		if (count) {
			ret = memcpy_s(buffer + sizeof(struct pkt_header), MAX_PKT_LENGTH - sizeof(struct pkt_header),
				       buf, count);
			if (EOK != ret) {
				pr_err("%s memcpy buffer fail, ret[%d]\n", __func__, ret);
				return -ENOMEM;
			}
		}
		winfo.wr_buf = buffer + sizeof(struct pkt_header);
		winfo.wr_len = (unsigned short)count;
		ret = write_customize_cmd(&winfo, rd, is_lock);
	} else {
		unsigned int *subcmd = (unsigned int *)(((struct pkt_header *)buffer) + 1);
		*subcmd = subtype;
		if (count > (MAX_PKT_LENGTH - CONTEXTHUB_HEADER_SIZE)) {
			pr_err("%s cnt1 error[%ld]\n", __func__, count);
			return -ENOMEM;
		}

		if (count) {
			ret = memcpy_s(buffer + CONTEXTHUB_HEADER_SIZE, MAX_PKT_LENGTH - CONTEXTHUB_HEADER_SIZE, buf, count);
			if (EOK != ret) {
				pr_err("%s memcpy buffer fail, ret[%d]\n", __func__, ret);
				return -ENOMEM;
			}
		}
		winfo.wr_buf = buffer + sizeof(struct pkt_header);
		winfo.wr_len = (unsigned short)(count + sizeof(unsigned int));
		ret = write_customize_cmd(&winfo, rd, is_lock);
	}

	if (ret) {
		pr_err("%s error\n", __func__);
	}

	return ret;
}

/*
 * 发送IPC消息给contexthub
 */
int send_cmd_from_kernel(unsigned char cmd_tag, unsigned char cmd_type,
    unsigned int subtype, const char *buf, size_t count)
{
	return send_cmd_format(cmd_tag, cmd_type, subtype, buf, count, true, NULL);
}

/*
 * 发送IPC消息给contexthub nolock
 */
int send_cmd_from_kernel_nolock(unsigned char cmd_tag, unsigned char cmd_type,
    unsigned int subtype, const char *buf, size_t count)
{
	return send_cmd_format(cmd_tag, cmd_type, subtype, buf, count, false, NULL);
}

/*
 * 发送IPC消息给contexthub 并等待回复
 */
int send_cmd_from_kernel_response(unsigned char cmd_tag, unsigned char cmd_type,
    unsigned int subtype, const char *buf, size_t count, struct read_info *rd)
{
	return send_cmd_format(cmd_tag, cmd_type, subtype, buf, count, true, rd);
}
