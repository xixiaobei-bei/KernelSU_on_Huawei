/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
* Description:  ivp noc register
* Author:  donghuibin
* Create:  donghuibin
*/
#include "ivp_rdr.h"

#include <linux/io.h>
#include <linux/stat.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/printk.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/hisi/hisi_noc_modid_para.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/rdr_hisi_platform.h>

#include "mntn_subtype_exception.h"
#include "libhwsecurec/securec.h"
#include "soc_mid.h"
#include "ivp_core.h"
#include "ivp_log.h"

#define MS_DIV_US            (1000)
#define RDR_RESET_DELAY      (60000)   /* 60 s delay */

#define TARGET_FLOW_DEFAULT  (0xff)

enum RDR_IVP_EXC_TYPE {
    MODID_IVP_START = HISI_BB_MOD_IVP_START,
    MODID_IVP_EXC_NOC_1 = MODID_IVP_START,
    MODID_IVP_EXC_NOC_2,
    MODID_IVP_EXC_NOC_3,
    MODID_IVP_EXC_SMMU,
    MODID_IVP_EXC_END = HISI_BB_MOD_IVP_END,
} rdr_ivp_exc_t;

struct rdr_ivp_device {
    struct ivp_device *pivpdev;
} rdr_ivp_dev;

static struct noc_err_para_s ivp_noc_para[] = {
    {
        .masterid = (u32)SOC_IVP32_DSP_DSP_CORE_DATA_MID,
        .targetflow = TARGET_FLOW_DEFAULT,
        .bus = NOC_ERRBUS_VCODEC,
    },
    {
        .masterid = (u32)SOC_IVP32_DSP_DSP_CORE_INST_MID,
        .targetflow = TARGET_FLOW_DEFAULT,
        .bus = NOC_ERRBUS_VCODEC,
    },
    {
        .masterid = (u32)SOC_IVP32_DSP_DSP_DMA_MID,
        .targetflow = TARGET_FLOW_DEFAULT,
        .bus = NOC_ERRBUS_VCODEC,
    },
};

static struct rdr_exception_info_s ivp_exc_info[] = {
    {
        .e_modid = (u32)MODID_IVP_EXC_NOC_1,
        .e_modid_end = (u32)MODID_IVP_EXC_NOC_3,
        .e_process_priority = RDR_ERR,
        .e_reboot_priority = RDR_REBOOT_NO,
        .e_notify_core_mask = RDR_IVP,
        .e_reset_core_mask = RDR_IVP,
        .e_from_core = RDR_IVP,
        .e_reentrant = (u32)RDR_REENTRANT_DISALLOW,
        .e_exce_type = IVP_S_EXCEPTION,
        .e_exce_subtype = IVP_NOC_ERR,
        .e_upload_flag = (u32)RDR_UPLOAD_YES,
        .e_from_module = "IVP",
        .e_desc = "RDR IVP NOC",
    },
};

static struct rdr_ivp_device g_ivp_rdr_dev;

static void ivp_rdr_dump(u32 modid, u32 etype, u64 coreid,
    char *pathname, pfn_cb_dump_done pfn_cb)
{
    struct rdr_ivp_device *dev = &g_ivp_rdr_dev;

    if (pathname == NULL) {
        ivp_err("%s:pointer is NULL !!\n",  __func__);
        return;
    }
    ivp_err(" =========ivp noc happen==========\n");
    ivp_err(" modid:          [0x%x]\n",   modid);
    ivp_err(" coreid:         [0x%llx]\n", coreid);
    ivp_err(" exce tpye:      [0x%x]\n",   etype);
    ivp_err(" path name:      [%s]\n",     pathname);

    ivp_dump_status(dev->pivpdev);
    ivp_err(" =========ivp dump exit==========\n");
    if (pfn_cb) {
        pfn_cb(modid, coreid);
    }

    return;

}

static void ivp_rdr_reset(u32 modid, u32 etype, u64 coreid)
{

    return;
}

int ivp_noc_register(void)
{
    unsigned int i;
    for (i = 0; i < ARRAY_SIZE(ivp_noc_para); i++) {
        ivp_info("%s: register noc err, , type:%x", __func__, ((u32)MODID_IVP_EXC_NOC_1 + i));
        noc_modid_register(ivp_noc_para[i], ((u32)MODID_IVP_EXC_NOC_1 + i));
    }

    return 0;
}

static int ivp_rdr_module_register(void)
{
    struct rdr_module_ops_pub module_ops;
    struct rdr_register_module_result ret_info;
    int ret;

    ivp_info("%s: enter.\n", __func__);
    module_ops.ops_dump = ivp_rdr_dump;
    module_ops.ops_reset = ivp_rdr_reset;

    ret = rdr_register_module_ops(RDR_IVP, &module_ops, &ret_info);
    if (ret != 0) {
        ivp_err("%s: rdr_register_module_ops failed! return %d\n",  __func__, ret);
    }

    return ret;
}

static int ivp_rdr_exception_register(void)
{
    unsigned int i;
    unsigned int ret;

    ivp_info("%s: enter.\n", __func__);
    for (i = 0; i < ARRAY_SIZE(ivp_exc_info); i++) {
        ivp_info("%s: register rdr exception, i = %d, type:%d", __func__, i, ivp_exc_info[i].e_exce_type);

        ret = rdr_register_exception(&ivp_exc_info[i]);
        if (ret != ivp_exc_info[i].e_modid_end) {
            ivp_err("%s: rdr_register_exception failed, ret.%d.\n", __func__, ret);
            return -EINVAL;
        }
    }

    return 0;
}


int ivp_rdr_init(struct ivp_device *pdev)
{
    int ret;
    struct rdr_ivp_device *dev = &g_ivp_rdr_dev;

    ivp_info("[%s] enter", __func__);

    dev->pivpdev = pdev;
    ret = ivp_rdr_module_register();
    if (ret != 0) {
        ivp_err("%s: ivp_rdr_module_register failed.\n", __func__);
        return ret;
    }

    ret = ivp_rdr_exception_register();
    if (ret != 0) {
        ivp_err("%s: ivp_rdr_exception_register failed.\n", __func__);
        return ret;
    }

    ret = ivp_noc_register();
    if (ret != 0) {
        ivp_err("%s: ivp_noc_register register failed.\n", __func__);
        return ret;
    }

    return ret;
}

int ivp_rdr_deinit(void)
{

    return 0;
}
