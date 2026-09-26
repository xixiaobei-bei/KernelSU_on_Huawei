#include <product_config.h>
#include <bsp_print.h>
#include <bsp_icc.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/semaphore.h>

#undef THIS_MODU
#define THIS_MODU mod_avs

#define avs_err(fmt, ...) bsp_err(fmt, ##__VA_ARGS__)
#define VTRIM_NUM 6

struct avs_msg {
    u32 ccpu_profile;
    u32 fastbus_freq;
    u32 slowbus_freq;
    u32 dsp_freq;
    u32 bbp_profile;
    u32 volt;
};

struct semaphore g_avs_sem_lr;
struct semaphore g_avs_sem_nr;

u32 g_avs_vtrim_lr[VTRIM_NUM] = { 0, 1, 2, 3, 4, 5 };
u32 g_avs_vtrim_nr[VTRIM_NUM] = { 0, 1, 2, 3, 4, 5 };
u32 g_avs_num = VTRIM_NUM;

module_param_array(g_avs_vtrim_lr, uint, &g_avs_num, S_IRUGO);
MODULE_PARM_DESC(g_avs_vtrim_lr, "g_avs_vtrim_lr");
module_param_array(g_avs_vtrim_nr, uint, &g_avs_num, S_IRUGO);
MODULE_PARM_DESC(g_avs_vtrim_nr, "g_avs_vtrim_nr");

int avs_send_msg_lr(u32 ccpu_profile, u32 fastbus_freq, u32 slowbus_freq, u32 dsp_freq, u32 bbp_profile, u32 volt)
{
    struct avs_msg msg;
    int msg_size = sizeof(struct avs_msg);
    u32 channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_AVS_TEST;
    int ret;

    msg.ccpu_profile = ccpu_profile;
    msg.fastbus_freq = fastbus_freq;
    msg.slowbus_freq = slowbus_freq;
    msg.dsp_freq = dsp_freq;
    msg.bbp_profile = bbp_profile;
    msg.volt = volt;

    ret = bsp_icc_send(ICC_CPU_MODEM, channel_id, (unsigned char *)&msg, msg_size);
    if (ret != msg_size) {
        avs_err("icc msgsize=%d, ret=%d\n", msg_size, ret);
    }

    down(&g_avs_sem_lr);
    return 0;
}
EXPORT_SYMBOL(avs_send_msg_lr);

int avs_send_msg_nr(u32 ccpu_profile, u32 fastbus_freq, u32 slowbus_freq, u32 dsp_freq, u32 bbp_profile, u32 volt)
{
    struct avs_msg msg;
    int msg_size = sizeof(struct avs_msg);
    u32 channel_id = ICC_CHN_NRIFC << 16 | NRIFC_RECV_FUNC_AVS_TEST;
    int ret;

    msg.ccpu_profile = ccpu_profile;
    msg.fastbus_freq = fastbus_freq;
    msg.slowbus_freq = slowbus_freq;
    msg.dsp_freq = dsp_freq;
    msg.bbp_profile = bbp_profile;
    msg.volt = volt;

    ret = bsp_icc_send(ICC_CPU_NRCCPU, channel_id, (unsigned char *)&msg, msg_size);
    if (ret != msg_size) {
        avs_err("icc msgsize=%d, ret=%d\n", msg_size, ret);
    }

    down(&g_avs_sem_nr);
    return 0;
}
EXPORT_SYMBOL(avs_send_msg_nr);

int avs_icc_read_cb_lr(u32 channel_id, u32 len, void *context)
{
    int ret;
    int msg_size = VTRIM_NUM * sizeof(u32);

    ret = bsp_icc_read(channel_id, (unsigned char *)g_avs_vtrim_lr, msg_size);
    if (ret != msg_size) {
        avs_err("acpu avs icc readcb size=%d ret=%d\n", msg_size, ret);
        return ICC_ERR;
    }
    avs_err("recv msg from lrcore\n");
    up(&g_avs_sem_lr);

    return 0;
}
int avs_icc_read_cb_nr(u32 channel_id, u32 len, void *context)
{
    int ret;
    int msg_size = VTRIM_NUM * sizeof(u32);

    ret = bsp_icc_read(channel_id, (unsigned char *)g_avs_vtrim_nr, msg_size);
    if (ret != msg_size) {
        avs_err("acpu avs icc readcb size=%d ret=%d\n", msg_size, ret);
        return ICC_ERR;
    }
    avs_err("recv msg from nrcore\n");
    up(&g_avs_sem_nr);

    return 0;
}

int avs_test_init(void)
{
    u32 channel_id_lr = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_AVS_TEST;
    u32 channel_id_nr = ICC_CHN_NRIFC << 16 | NRIFC_RECV_FUNC_AVS_TEST;
    int ret;

    sema_init(&g_avs_sem_lr, 0);
    sema_init(&g_avs_sem_nr, 0);

    ret = bsp_icc_event_register(channel_id_lr, avs_icc_read_cb_lr, NULL, (write_cb_func)NULL, (void *)NULL);
    if (ret != ICC_OK) {
        avs_err("lr icc init err\n");
    }

    ret = bsp_icc_event_register(channel_id_nr, avs_icc_read_cb_nr, NULL, (write_cb_func)NULL, (void *)NULL);
    if (ret != ICC_OK) {
        avs_err("nr icc init err\n");
    }

    avs_err("avs_test_init ok\n");
    return 0;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
subsys_initcall(avs_test_init);
#endif
