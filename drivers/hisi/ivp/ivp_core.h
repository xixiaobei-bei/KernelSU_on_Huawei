#ifndef _IVP_CORE_H_
#define _IVP_CORE_H_

#include "ivp_platform.h"

struct file_header {
	char name[4];
	char time[20];
	unsigned int image_size;
	unsigned int sect_count;
};

struct image_section_header {
	unsigned short index;
	unsigned char type;
	unsigned char attribute;
	unsigned int offset;
	unsigned int vaddr;
	unsigned int size;
};

u32 ivp_reg_read(unsigned int off);
void ivp_hw_runstall(struct ivp_device *devp, int mode);
void ivp_reg_write(unsigned int off, u32 val);
u32 ivp_wdg_reg_read(unsigned int off);
void ivp_wdg_reg_write(unsigned int off, u32 val);
u32 ivp_smmu_reg_read(unsigned int off);
u32 ivp_pctrl_reg_read(unsigned int off);
int ivp_change_clk(struct ivp_device *ivp_devp, unsigned int level);
bool is_ivp_in_secmode(void);
void ivp_dump_status(struct ivp_device *pdev);
void *ivp_vmap(phys_addr_t paddr, size_t size, unsigned int *offset);

extern struct mutex ivp_sec_mem_mutex;
extern struct mutex ivp_power_up_off_mutex;

#endif /* _IVP_CORE_H_ */
