 /*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <linux/netdevice.h>
#include <linux/module.h>
#include <net/genetlink.h>
#include <securec.h>
#include <bsp_print.h>
#include <osl_malloc.h>

#include "hmi_core.h"
#include "hmi_chan_adp.h"
#include "hmi_common.h"

#define HMI_GENL_NAME "hmi_genl"
#define HMI_GENL_VERSION 1
#define HMI_NETLINK_HDR_LEN (NLMSG_HDRLEN + GENL_HDRLEN)

enum hmi_cmd {
    HMI_CMD_UNSPEC,
    HMI_CMD_RCV_MSG,
};

enum hmi_service_id {
    HMI_SERVICEID_KIT,
    HMI_SERVICEID_VCOM,
};

struct hmi_core g_hmi_core_ctx;
struct genl_family g_hmi_genl_family;

struct hmi_header {
    unsigned char version;
    unsigned char service_id;
    unsigned char msg_type: 2;
    unsigned char frag_index: 4;
    unsigned char end_of_frag: 1;
    unsigned char frag_flag: 1;
    unsigned char flag;
    unsigned short len;
    unsigned short rsv;
    unsigned int trans_id;
};
#define HMI_HEADER_LEN sizeof(struct hmi_header)

struct hmi_core *hmi_get_core_ctx(void)
{
    return &g_hmi_core_ctx;
}

void hmi_header_report(unsigned char* data)
{
    struct hmi_header * header = (struct hmi_header *)data;

    bsp_info("HMI_HEADER: version = %u service_id = %u msg_type = %u trans_id = %u len = %u",
        header->version, header->service_id, header->msg_type, header->trans_id, header->len);
}

static int hmi_genl_recv_msg(struct sk_buff *skb, struct genl_info *info)
{
    struct hmi_core *hmi_core_ctx = &g_hmi_core_ctx;
    struct nlmsghdr *nlhdr = NULL;
    struct genlmsghdr *genlhdr = NULL;
    unsigned char *msghdr = NULL;
    unsigned int msg_len;
    int ret;

    if (skb == NULL || info == NULL) {
        hmi_core_ctx->stat.recv_skb_null++;
        return -1;
    }

    nlhdr = nlmsg_hdr(skb);
    if (nlhdr == NULL) {
        hmi_core_ctx->stat.recv_nlhdr_null++;
        return -1;
    }

    genlhdr = nlmsg_data(nlhdr);
    msghdr = genlmsg_data(genlhdr);

    if ((skb->len < HMI_NETLINK_HDR_LEN) ||
        (skb->len > (HMI_MAXMSGLEN + HMI_NETLINK_HDR_LEN))) {
        hmi_core_ctx->stat.invalid_msg_len++;
        return -1;
    }
    msg_len = skb->len - HMI_NETLINK_HDR_LEN;

    hmi_core_ctx->port_id = info->snd_portid;
    hmi_core_ctx->net = genl_info_net(info);

    ret = hmi_chan_adp_send(msghdr, msg_len);
    if (ret != 0) {
        hmi_core_ctx->stat.mci_write_failed++;
        return -1;
    }

    hmi_header_report(msghdr);
    hmi_core_ctx->stat.recv_msg_succ++;

    return 0;
}

int hmi_genl_reply_msg(unsigned char *buf, int len)
{
    struct hmi_core *hmi_core_ctx = &g_hmi_core_ctx;
    struct genl_family *family = &g_hmi_genl_family;
    struct sk_buff *skb = NULL;
    void *payload_hdr = NULL;
    int ret;

    if (buf == NULL || len <= 0 || len > NLMSG_DEFAULT_SIZE - HMI_NETLINK_HDR_LEN) {
        hmi_core_ctx->stat.reply_invalid_para++;
        return -1;
    }

    hmi_header_report(buf);

    skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (skb == NULL) {
        hmi_core_ctx->stat.msg_alloc_failed++;
        return -1;
    }

    payload_hdr = genlmsg_put(skb, hmi_core_ctx->port_id, 0, family, 0, HMI_CMD_RCV_MSG);
    if (payload_hdr == NULL) {
        hmi_core_ctx->stat.msg_put_failed++;
        goto err;
    }

    ret = memcpy_s((char*)payload_hdr, NLMSG_DEFAULT_SIZE - HMI_NETLINK_HDR_LEN,
        buf, len);
    if (ret != EOK) {
        hmi_core_ctx->stat.payload_copy_failed++;
        goto err;
    }

    skb_put(skb, len);
    genlmsg_end(skb, payload_hdr);

    genlmsg_unicast(hmi_core_ctx->net, skb, hmi_core_ctx->port_id);

    hmi_core_ctx->stat.reply_succ++;
    return 0;
err:
    dev_kfree_skb_any(skb);
    return -1;
}

int hmi_recv_data_parse(unsigned char *recv_msg, int recv_len)
{
    int ret;
    int read_buf_len;
    struct hmi_header *header = NULL;
    struct hmi_core *ctx = &g_hmi_core_ctx;

    for (read_buf_len = recv_len; read_buf_len > 0 && recv_msg != NULL;) {
        if (read_buf_len < HMI_HEADER_LEN) {
            bsp_err("recv_msg size err, size = %d\n", read_buf_len);
            return -1;
        }
        header = (struct hmi_header *)recv_msg;
        if ((header->len > read_buf_len - HMI_HEADER_LEN) || (header->len + HMI_HEADER_LEN > HMI_MAXMSGLEN)) {
            ctx->stat.data_parse_invalid_para++;
            return -1;
        }

        ret = hmi_genl_reply_msg(recv_msg, header->len + HMI_HEADER_LEN);
        if (ret != 0) {
            bsp_err("hmi_core reply msg failed\n");
            ctx->stat.data_parse_reply_msg_failed++;
            return -1;
        }
        ctx->stat.data_parse_pkt_succ++;
        read_buf_len -= (header->len + HMI_HEADER_LEN);
        recv_msg += (header->len + HMI_HEADER_LEN);
    }

    return 0;
}

void hmi_hds_report(void)
{
    struct hmi_core *ctx = &g_hmi_core_ctx;
    int ret;
    int core_stat_len = sizeof(ctx->stat);

    ret = memcpy_s(ctx->hds_info, ctx->hds_info_len, &ctx->stat, core_stat_len);
    if (ret != EOK) {
        bsp_err("hmi_core stat report failed");
        return;
    }

    hmi_report_chan_adp_info((unsigned char *)ctx->hds_info + core_stat_len,
        ctx->hds_info_len - core_stat_len);

    return;
}

void hmi_report_init(struct hmi_core *ctx)
{
    struct trans_report_info *info = &ctx->report_info;
    int report_len;
    int ret;

    ret = hmi_get_chan_adp_om_length();
    if (ret <= 0) {
        bsp_err("get_chan_adp_om_length failed");
        return;
    }

    report_len = sizeof(struct hmi_core_statics) + ret;
    ctx->hds_info = osl_malloc(report_len);
    if (ctx->hds_info == NULL) {
        bsp_err("hds_info alloc failed");
        return;
    }

    info->ul_msg_id = TRANS_MSGID_HMI;
    info->data = ctx->hds_info;
    info->size = report_len;
    info->func_cb = hmi_hds_report;
    INIT_LIST_HEAD(&info->list);

    ret = bsp_trans_report_register(info);
    if (ret) {
        bsp_err("trans report register fail\n");
    }

    ctx->hds_info_len = report_len;
    return;
}

static const struct genl_ops g_hmi_genl_ops[] = {
    {
        .cmd = HMI_CMD_RCV_MSG,
        .doit = hmi_genl_recv_msg,
    },
};

struct genl_family g_hmi_genl_family = {
    .hdrsize = 0,
    .name = HMI_GENL_NAME,
    .version = HMI_GENL_VERSION,
    .ops = g_hmi_genl_ops,
    .n_ops = ARRAY_SIZE(g_hmi_genl_ops),
};

int hmi_init(void)
{
    struct hmi_core *ctx = &g_hmi_core_ctx;
    int ret;

    bsp_err("init_start\n");

    ret = genl_register_family(&g_hmi_genl_family);
    if (ret != 0) {
        bsp_err("genl_register_family failed\n");
        return -1;
    }

    ret = hmi_chan_adp_init();
    if (ret != 0) {
        bsp_err("hmi_chan_adp_init failed\n");
        return -1;
    }

    ret = hmi_register_chan_adp_read_cb(hmi_recv_data_parse);
    if (ret != 0) {
        bsp_err("hmi_register_chan_adp_read_cb failed\n");
        return -1;
    }

    hmi_report_init(ctx);

    bsp_err("init_end\n");

    return 0;
}

int hmi_genl_reply_test(unsigned char *buf, int len)
{
    int ret;

    ret = hmi_chan_adp_send(buf, len);
    if (ret != 0) {
        bsp_err("hmi_reply test adp send fail\n");
        return -1;
    }

    bsp_err("hmi_reply test adp send succ\n");
    return 0;
}

int hmi_test_acore(void)
{
    int ret;

    ret = hmi_register_chan_adp_read_cb(hmi_genl_reply_test);
    if (ret != 0) {
        bsp_err("hmi_test_acore chan_adp_read_cb register fail\n");
        return -1;
    }

    return 0;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(hmi_init);
#endif
