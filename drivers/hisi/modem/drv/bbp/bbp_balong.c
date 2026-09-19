#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>

#include <bsp_dump.h>
#include <bsp_pm_om.h>
#include <bsp_sysctrl.h>

#include <hi_bbp.h>
#include <bsp_print.h>

#undef THIS_MODU
#define THIS_MODU mod_bbp

void* g_bbp_acore_om_info_addr = NULL;
dump_handle g_bbp_acore_dump_handle;
u32 bbpom_cmd[]={0x1100,0x1001,0x1002,0x1003,0x1004,0x1005,0x1200,0x1400,0x1600,0x0,0x200,0x400,0x600,0x800,0xa00,0xc00,0xe00};
u32 bbpom_apscreg[]={0xac,0xb4,0xb8,0x168,0x16c,0x1b8,0x208,0x240,0x30c,0x324,0x360,0x530,0x534,0x538,0x53c,0x540};

void balong_bbp_get_om_info(int print_enable)
{
    u32 i;
    u32 value;
    void *pctrl_addr;
    void *sctrl_addr;
    u32 cmd_num = sizeof(bbpom_cmd)/sizeof(bbpom_cmd[0]);
    u32 reg_num = sizeof(bbpom_apscreg)/sizeof(bbpom_apscreg[0]);

    bsp_err("balong_bbp_dump_hook in...\n");

    pctrl_addr = ioremap(BBP_PCTRL_OM_BASE_ADDR, BBP_PCTRL_OM_BASE_SIZE);
    if (!pctrl_addr) {
        bsp_err("fail to ioremap\n");
        return;
    }

    sctrl_addr = bsp_sysctrl_addr_byindex(sysctrl_ao);

    for (i = 0; i < cmd_num; i++) {
        writel(bbpom_cmd[i], sctrl_addr+BBP_PCTRL_OM_CTRL_OFFSET);
        value = (u32)readl(pctrl_addr+BBP_PCTRL_OM_DATA_OFFSET);
        if (g_bbp_acore_om_info_addr) {
            writel(value, g_bbp_acore_om_info_addr + (i + 1)*4);
        }

        if ((!g_bbp_acore_om_info_addr) || (print_enable)) {
            bsp_err("BBP om info, cmd: %08X, data:%08X\n", bbpom_cmd[i], value);
        }
    }

    for (i = 0; i < reg_num; i++) {
        value = (u32)readl(sctrl_addr + bbpom_apscreg[i]);
        if (g_bbp_acore_om_info_addr) {
            writel(value, g_bbp_acore_om_info_addr + (cmd_num + i + 1)*4);
        }
        if ((!g_bbp_acore_om_info_addr) || (print_enable)) {
            bsp_err("BBP om info, reg: %08X, data:%08X\n", bbpom_apscreg[i], value);
        }
    }

    iounmap(pctrl_addr);

    bsp_err("balong_bbp_dump_hook out...\n");
}

void balong_bbp_dump_hook(void)
{
    balong_bbp_get_om_info(0);
}

int __init balong_bbp_acore_init(void)
{
    u32 cmd_num = sizeof(bbpom_cmd)/sizeof(bbpom_cmd[0]);
    u32 reg_num = sizeof(bbpom_apscreg)/sizeof(bbpom_apscreg[0]);

    g_bbp_acore_dump_handle = bsp_dump_register_hook("BBP_ACore", balong_bbp_dump_hook);
    g_bbp_acore_om_info_addr = bsp_pm_dump_get(PM_OM_BBP, ((cmd_num) + reg_num + 2) * 4);
    if (g_bbp_acore_om_info_addr) {
        writel(0x50424241U, g_bbp_acore_om_info_addr);  /* Magic Number: ABBP */
    }

    return 0;
}

module_init(balong_bbp_acore_init);/*lint -e528*/

