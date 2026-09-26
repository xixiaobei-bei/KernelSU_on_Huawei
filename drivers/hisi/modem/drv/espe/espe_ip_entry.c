
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
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/jhash.h>
#include <linux/kernel.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <linux/netdevice.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_tuple.h>

#include <securec.h>
#include <bsp_espe.h>

#include "func/espe_direct_fw.h"
#include "espe_ip_entry.h"
#include "espe_dbg.h"


/*
 * debug code for spe_ip_fw_add, the IP_FW_ADD_ERR_REC_MAX is the max code line
 * of function: spe_ip_fw_add
 */
#define IP_FW_ADD_ERR_REC_MAX 300
#define IP_FW_ADD_REC_MASK (IP_FW_ADD_ERR_REC_MAX - 1)
#define IP_FW_ADD_ERR_REC_INIT() do { \
    /* idx 0 is record the base line */                                              \
    if (unlikely(NULL == g_sg_ip_fw_err_line)) {                                     \
        g_sg_ip_fw_err_line = kzalloc(g_sg_ip_fw_dft_max * sizeof(int), GFP_ATOMIC); \
        if (g_sg_ip_fw_err_line != NULL)                                             \
            g_sg_ip_fw_err_line[0] = __LINE__;                                       \
    }                                                                                \
} while (0)

#define IP_FW_ADD_ERR_REC_START() do { \
    int idx;                                                                   \
    /* the room of "err line rec" is ok */                                     \
    if (likely(g_sg_ip_fw_err_line != NULL)) {                                 \
        idx = __LINE__ - g_sg_ip_fw_err_line[0];                               \
        /* the room of "err line rec" is not enough */                         \
        if (unlikely(idx >= g_sg_ip_fw_dft_max)) {                             \
            /* expand the max and remalloc */                                  \
            int *rec_alloc;                                                    \
            g_sg_ip_fw_dft_max = idx + 100;                                    \
            rec_alloc = kzalloc(g_sg_ip_fw_dft_max * sizeof(int), GFP_ATOMIC); \
            if (rec_alloc != NULL) {                                           \
                /* kfree the old one and use the new room */                   \
                rec_alloc[0] = g_sg_ip_fw_err_line[0];                         \
                kfree((void *)g_sg_ip_fw_err_line);                            \
                g_sg_ip_fw_err_line = rec_alloc;                               \
                g_sg_ip_fw_err_line[idx]++;                                    \
            }                                                                  \
        } /* ok, record it */                                                  \
        else {                                                                 \
            g_sg_ip_fw_err_line[idx]++;                                        \
        }                                                                      \
    }                                                                          \
} while (0)


int bsp_espe_update_ipfw_entry(struct wan_dev_info_s *wan_dev_info)
{
    struct spe *spe = &g_espe;
    int ret = 0;

    if (spe->direct_fw.dfw_flags.enable) {
        ret = espe_direct_fw_record_wandev(wan_dev_info);
    }
    return ret;
}


void espe_ip_fw_htab_set(void)
{
    struct spe *spe = &g_espe;
    unsigned int size;

    size = sizeof(struct espe_ip_fw_entry);
    size /= 0x4;

    spe_writel(spe->regs, SPE_HASH_BADDR_L, lower_32_bits(spe->ipfw.hbucket_dma));
    spe_writel(spe->regs, SPE_HASH_BADDR_H, upper_32_bits(spe->ipfw.hbucket_dma));
    spe_writel(spe->regs, SPE_HASH_WIDTH, size);
    spe_writel(spe->regs, SPE_HASH_DEPTH, spe->ipfw.hlist_size);
    spe_writel(spe->regs, SPE_HASH_ZONE, spe->ipfw.hzone);
    spe_writel(spe->regs, SPE_HASH_RAND, spe->ipfw.hrand);
    // we are going to use default reg setting on HI_SPE_HASH_L3_PROTO_OFFSET;
}
static void espe_ip_empty_entry_free(struct spe *spe, unsigned int size_ipfw)
{
    if (spe->ipfw.hbucket_empty) {
        dma_free_coherent(spe->dev, size_ipfw * SPE_IPFW_HTABLE_EMPTY_SIZE, spe->ipfw.hbucket_empty,
                          spe->ipfw.hbucket_dma_empty);
        spe->ipfw.hbucket_empty = NULL;
    }
    return;
}
static void espe_ip_entry_free(struct spe *spe, unsigned int size_ipfw)
{
    int i = 0;
    struct espe_ip_fw_entry *entry = NULL;

    while (i < spe->ipfw.hlist_size) {
        entry = (struct espe_ip_fw_entry *)(spe->ipfw.hbucket + i * size_ipfw);
        if (entry->priv != NULL) {
            kfree(entry->priv);
        }
        i++;
    }

    if (spe->ipfw.hbucket) {
        dma_free_coherent(spe->dev, size_ipfw * spe->ipfw.hlist_size, spe->ipfw.hbucket, spe->ipfw.hbucket_dma);

        spe->ipfw.hbucket = NULL;
    }
    return;
}
int espe_ip_empty_entry_init(struct spe *spe, unsigned int size_ipfw)
{
    struct espe_ip_fw_entry *empty_entry = NULL;
    int ret;

    /* alloc spe hash bucket empty for spe switch of func */
    empty_entry = dma_alloc_coherent(spe->dev, size_ipfw * SPE_IPFW_HTABLE_EMPTY_SIZE, &spe->ipfw.hbucket_dma_empty,
                                     GFP_KERNEL);
    if (empty_entry == NULL) {
        SPE_ERR("alloc ip fw hbucket empty 1 entry failed\n");
        return -ENOMEM;
    }
    spe->ipfw.hbucket_empty = empty_entry;

    ret = memset_s(empty_entry, sizeof(*empty_entry) * SPE_IPFW_HTABLE_EMPTY_SIZE, 0,
                   size_ipfw * SPE_IPFW_HTABLE_EMPTY_SIZE);
    if (ret) {
        SPE_ERR("memset_s ip fw hbucket empty  entry failed\n");
        espe_ip_empty_entry_free(spe, size_ipfw);
    }
    return ret;
}

static inline void espe_ip_entry_defcfg(struct spe *spe, unsigned int size_ipfw)
{
    if (!spe->ipfw.hlist_size) {
        spe->ipfw.hlist_size = SPE_IPFW_HTABLE_SIZE;
    }

    spe->ipfw.hrand = SPE_IPFW_HTABLE_RAND;
    spe->ipfw.hzone = SPE_IPFW_HTABLE_ZONE;
    spe->ipfw.hitem_width = size_ipfw; /* jiffies */
    return;
}

int espe_ip_entry_table_init(struct spe *spe)
{
    int ret;
    int i;
    unsigned int size_ipfw = sizeof(struct espe_ip_fw_entry);

    espe_ip_entry_defcfg(spe, size_ipfw);
    ret = espe_ip_empty_entry_init(spe, size_ipfw);
    if (ret) {
        return ret;
    }

    /* alloc spe hash bucket */
    spe->ipfw.hbucket = dma_alloc_coherent(spe->dev, size_ipfw * spe->ipfw.hlist_size, &spe->ipfw.hbucket_dma,
                                           GFP_KERNEL);
    if (!spe->ipfw.hbucket) {
        SPE_ERR("alloc ip fw hbucket of %d entrys failed\n", spe->ipfw.hlist_size);
        goto err_alloc_hbucket;
    }

    ret = memset_s(spe->ipfw.hbucket, sizeof(struct espe_ip_fw_entry) * spe->ipfw.hlist_size, 0,
                   size_ipfw * spe->ipfw.hlist_size);
    if (ret) {
        goto err_alloc_hbucket;
    }

    for (i = 0; i < spe->ipfw.hlist_size; i++) {
        struct espe_ip_fw_entry *entry = (struct espe_ip_fw_entry *)(spe->ipfw.hbucket + i * size_ipfw);

        entry->dead_timestamp = jiffies;
        entry->valid = 0;
        entry->priv = kzalloc(sizeof(struct espe_ip_fw_entry_priv), GFP_KERNEL);
        if (entry->priv == NULL) {
            SPE_ERR("alloc ip fw hbucket priv failed\n");
            goto err_alloc_hslab;
        }
        entry->priv->entry = entry;
        entry->priv->dma = spe->ipfw.hbucket_dma + i * size_ipfw;
        INIT_LIST_HEAD(&entry->priv->list);
    }

    /* alloc dma pool for ip fw entry */
    spe->ipfw.hslab = (struct dma_pool *)dma_pool_create(dev_name(spe->dev), spe->dev, size_ipfw, 4, PAGE_SIZE);

    if (NULL == spe->ipfw.hslab) {
        goto err_alloc_hslab;
    }

    INIT_LIST_HEAD(&spe->ipfw.free_list);
    INIT_LIST_HEAD(&spe->ipfw.wan_entry_list);

    spin_lock_init(&spe->ipfw.lock);
    spin_lock_init(&spe->ipfw.free_lock);

    spe->ipfw.deadtime = 2; /* jiffies */
    return 0;

err_alloc_hslab:
    espe_ip_entry_free(spe, size_ipfw);
err_alloc_hbucket:
    espe_ip_empty_entry_free(spe, size_ipfw);
    return -1;
}

void espe_ip_entry_res_free(struct spe *spe)
{
    if (spe->ipfw.hslab != NULL) {
        dma_pool_destroy(spe->ipfw.hslab);
        spe->ipfw.hslab = NULL;
    }

    if (spe->ipfw.hbucket_empty != NULL) {
        dma_free_coherent(spe->dev, sizeof(struct espe_ip_fw_entry) * spe->ipfw.hlist_size, spe->ipfw.hbucket,
                          spe->ipfw.hbucket_dma);
        spe->ipfw.hbucket_empty = NULL;
    }

    if (spe->ipfw.hbucket != NULL) {
        dma_free_coherent(spe->dev, sizeof(struct espe_ip_fw_entry) * SPE_IPFW_HTABLE_EMPTY_SIZE,
                          spe->ipfw.hbucket_empty, spe->ipfw.hbucket_dma_empty);
        spe->ipfw.hbucket = NULL;
    }
    return;
}

void espe_ip_entry_table_exit(struct spe *spe)
{
    int i;
    unsigned long flags;
    dma_addr_t dma;
    struct espe_ip_fw_entry *head = NULL;
    struct espe_ip_fw_entry *pos = NULL;
    struct espe_ip_fw_entry_priv *n = NULL;
    struct espe_ip_fw_entry_priv *priv_pos = NULL;
    struct espe_ip_fw_entry_priv *priv_head = NULL;
    spin_lock_irqsave(&spe->ipfw.lock, flags);

    /* free node in free queue */
    for (i = 0; i < spe->ipfw.hlist_size; i++) {
        head = (struct espe_ip_fw_entry *)(spe->ipfw.hbucket + i * sizeof(struct espe_ip_fw_entry));
        priv_head = head->priv;
        list_for_each_entry_safe(priv_pos, n, &priv_head->list, list)
        {
            pos = priv_pos->entry;
            list_del_init(&pos->priv->list);
            list_del_init(&pos->priv->wan_list);
            dma = pos->priv->dma;
            kfree(pos->priv);
            dma_pool_free(spe->ipfw.hslab, pos, dma);
        }
    }

    spin_unlock_irqrestore(&spe->ipfw.lock, flags);

    spin_lock_irqsave(&spe->ipfw.free_lock, flags);

    list_for_each_entry_safe(priv_pos, n, &spe->ipfw.free_list, list)
    {
        pos = priv_pos->entry;
        dma = pos->priv->dma;
        kfree(pos->priv);
        dma_pool_free(spe->ipfw.hslab, pos, dma);
    }

    spin_unlock_irqrestore(&spe->ipfw.free_lock, flags);

    espe_ip_entry_res_free(spe);
    return;
}

MODULE_LICENSE("GPL");


