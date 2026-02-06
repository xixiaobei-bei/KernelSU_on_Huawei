#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <osl_module.h>
#include <osl_malloc.h>
#include <osl_sem.h>
#include <osl_bio.h>
#include <bsp_thermal.h>
#include <bsp_print.h>
#undef THIS_MODU
#define THIS_MODU  mod_thermal


int thermal_up_init(void)
{
    return 0;
}
#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(thermal_up_init);
#endif
