

#ifndef __CP_DUMP_AREA_H__
#define __CP_DUMP_AREA_H__

#include <product_config.h>
#include "mntn_interface.h"
#ifndef __ASSEMBLY__
#include "osl_types.h"
#include "osl_list.h"
#include "osl_spinlock.h"
#endif
#include "bsp_memmap.h"
#include "bsp_s_memory.h"
#include "bsp_dump_mem.h"


/*  dump content

    ---------------
   | head          |
    ---------------
   | area map [8]  |
    ---------------
   | area 1        |
    ---------------
   | area 2        |
    ---------------
   | area 3        |
    ---------------
   | area 4        |
    ---------------
   | area 5        |
    ---------------
   | area 6        |
    ---------------
   | area 7        |
    ---------------
   | area 8        |
    ---------------
*/

    /* field number supported by area */
#define DUMP_FIELD_MAX_NUM  64
#define DUMP_AREA_MAGICNUM  0x4e656464


#if defined(__KERNEL__)

#define MDMAP_AREA                (DUMP_AREA_MDMAP)
#define MDMAP_AREA_NAME           "MDMAP"
#define MDMAP_FIELD_ID_START      (0x01000000)
#define MDMAP_FIELD_ID_END        (0x01ffffff)

#endif

#define DUMP_MODEMAP_BASE_INFO_SIZE                 (0x180)
#define DUMP_MODEMAP_USER_DATA_SIZE                 (0x1000)
#define DUMP_MODEMAP_REG_SIZE                       (0x3000)
#define DUMP_MODEMAP_REG_SINGLE_SIZE                (0x1000)


struct dump_global_area_ctrl_s{
    u32                             init_state;
    u32                             length;
    struct dump_global_struct_s*    virt_addr;
    unsigned long                   phy_addr;
};

struct dump_nr_area_ctrl_s{
    u32                                       init_state;
    u32                                       length;
    struct dump_nr_level2_global_struct_s*    virt_addr;
    unsigned long                             phy_addr;
};


struct dump_field_ctrl_info_s
{
    dump_area_t*        virt_area_addr;
    void*               phy_area_addr;
    u32                 total_length;
    u32                 free_offset;
    u32                 free_length;
    u32                 init_flag;
    u32                 field_num;
    spinlock_t          lock;
};

void bsp_dump_save_self_addr(void);
void dump_fill_save_done(void);
s32 dump_get_lr_cp_save_done(void);
void dump_mdmap_field_init(void);
s32 dump_get_load_info(dump_load_info_t * load_info);
s32 dump_get_level1_area_info(DUMP_AREA_ID areaid,struct dump_area_mntn_addr_info_s* area_info);
s32 dump_area_init(void);
s32 dump_get_global_info(struct dump_global_area_ctrl_s * global_area);
void* dump_get_global_baseinfo(void);
s32 dump_get_level2_area_info(DUMP_LEVLE2_AREA_ID level2_area_id,struct dump_area_mntn_addr_info_s* area_info);
#ifdef MNTN_AREA_CBOOT_ADDR
void dump_clear_cpboot_area(void);
#else
static inline void dump_clear_cpboot_area(void){}
#endif
void* dump_get_mntn_base_addr(void);
void* dump_get_rdr_top_head(void);


#endif
