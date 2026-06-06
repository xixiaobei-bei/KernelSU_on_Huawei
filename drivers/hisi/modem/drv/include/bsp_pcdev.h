#ifndef __HISI_ADP_PCDEV__
#define __HISI_ADP_PCDEV__

enum pcdev_port_id_e{
    pcdev_ctrl = 0,
    pcdev_ttyGS0,
    pcdev_c_shell,
    pcdev_4g_diag,
    pcdev_3g_diag,
    pcdev_mdm,
    pcdev_gps,
    pcdev_a_shell,
    pcdev_err,
    pcdev_voice,
    pcdev_skytone,
    pcdev_agent_nv,
    pcdev_agent_om,
    pcdev_agent_msg,
    pcdev_appvcom,
    pcdev_modem,
    pcdev_pmom,
    pcdev_reserve,
    pcdev_port_bottom
};

void *bsp_pcdev_open(u32 dev_id);
s32 bsp_pcdev_close(void* handle);
s32 bsp_pcdev_write(void* handle, void *buf, u32 size);
s32 bsp_pcdev_read(void* handle, void *buf, u32 size);
s32 bsp_pcdev_ioctl(void* handle, u32 cmd, void *para);

void pcie_cdev_notify_enum_done(void);
void pcie_cdev_notify_disable(void);

#endif
