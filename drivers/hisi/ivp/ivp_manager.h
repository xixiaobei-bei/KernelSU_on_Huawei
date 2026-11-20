/*
 * This file defines api to cfg ivp core hardware to power up,
 * run, run stall, load firmware, etc
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _IVP_MAIN_H_
#define _IVP_MAIN_H_

#include "linux/irqreturn.h"
#include "ivp_platform.h"

#define IVP_WDG_REG_BASE_OFFSET          0x1000
#define IVP_SMMU_REG_BASE_OFFSET         0x40000
#define IVP_INIT_SUCCESS                 0x1234ABCD
#define IVP_INIT_RESULT_CHECK_TIMES      0x100
#define SMMU_RESET_WAIT_TIME             1000

#ifdef SEC_IVP_ENABLE
#define IS_CONST
#else
#define IS_CONST const
#endif

enum {
	IVP_BOOT_FROM_IRAM = 0,
	IVP_BOOT_FROM_DDR = 1,
};

enum {
	IVP_DISABLE = 0,
	IVP_ENABLE  = 1,
	IVP_MODE_INVALID = 2,
};

enum ip_core_type {
	IVP_CORE_TYPE = 0,
	IPC_CORE_TYPE = 1,
	CORE_TYPE_INVALID = 2,
};

u32 ivp_reg_read(struct ivp_device *pdev, unsigned int off);
void ivp_reg_write(struct ivp_device *pdev, unsigned int off, u32 val);
void *ivp_vmap(phys_addr_t paddr, size_t size, unsigned int *offset);
bool is_ivp_in_secmode(struct ivp_device *pdev);

void ivp_dev_set_dynamic_clk(struct ivp_device *pdev, unsigned int mode);
void ivp_dump_status(struct ivp_device *pdev);
void ivp_hw_clr_wdg_irq(struct ivp_device *pdev);
void ivp_hw_set_ocdhalt_on_reset(struct ivp_device *pdev, int mode);
void ivp_hw_set_bootmode(struct ivp_device *pdev, int mode);
void ivp_hw_clockgate(struct ivp_device *pdev, int state);
void ivp_hw_disable_reset(struct ivp_device *pdev);
void ivp_hw_runstall(struct ivp_device *pdev, int mode);
int ivp_hw_query_runstall(struct ivp_device *pdev);
void ivp_hw_trigger_nmi(struct ivp_device *pdev);
int ivp_hw_query_waitmode(struct ivp_device *pdev);
void ivp_dev_stop(struct ivp_device *pdev);
long ivp_dev_suspend(struct ivp_device *pdev);
void ivp_dev_resume(struct ivp_device *pdev);
void ivp_dev_run(struct ivp_device *pdev);
int ivp_dev_keep_on_wdg(struct ivp_device *pdev);
void ivp_dev_sleep_wdg(struct ivp_device *pdev);
int ivp_dev_smmu_reset(struct ivp_device *pdev);
int ivp_dev_smmu_init(struct ivp_device *pdev);
int ivp_dev_smmu_deinit(struct ivp_device *pdev);
irqreturn_t ivp_wdg_irq_handler(int irq, void *dev_id);
irqreturn_t ivp_dwaxi_irq_handler(int irq, void *dev_id);
int ivp_setup_smmu_dev(struct ivp_device *pdev);
void ivp_release_iores(struct platform_device *plat_devp);
int ivp_init_reg_res(struct platform_device *plat_dev, struct ivp_device *pdev);
int ivp_setup_irq(struct platform_device *plat_devp, struct ivp_device *pdev);

int ivp_setup_dwaxi_irq(struct platform_device *plat_devp,
	struct ivp_device *pdev);
int ivp_setup_onchipmem_sections(struct platform_device *plat_devp,
	struct ivp_device *pdev);
#ifdef CONFIG_IVP_SMMU
int ivp_dev_smmu_invalid_tlb(struct ivp_device *pdev);
#endif
int ivp_init_pri(struct platform_device *plat_dev, struct ivp_device *ivp_devp);
void ivp_deinit_pri(struct ivp_device *ivp_devp);
int ivp_init_resethandler(struct ivp_device *ivp_devp);
void ivp_deinit_resethandler(struct ivp_device *ivp_devp);
int ivp_check_resethandler(IS_CONST struct ivp_device *ivp_devp);
int ivp_sec_loadimage(IS_CONST struct ivp_device *ivp_devp);
void ivp_dev_hwa_enable(struct ivp_device *ivp_devp);
void ivp_hw_enable_reset(struct ivp_device *ivp_devp);
bool ivp_check_coreid(enum ivp_core_id core_id);
enum ivp_core_id ivp_determine_coreid(struct platform_device *pdev,
	enum ip_core_type ip);
#endif /* _IVP_CORE_H_ */

