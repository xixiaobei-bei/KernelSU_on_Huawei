

#ifndef __OAL_HCC_BUS_EXCEPTION_H
#define __OAL_HCC_BUS_EXCEPTION_H

/* 其他头文件包含 */
#include "plat_type.h"
#include "plat_exception_rst.h"

/* 宏定义 */
/* for 1103 */
/* mpw2 config **/
#define WCPU_1103_MPW2_PANIC_MEMDUMP_MAX_SIZE 0x3c00 /* 15KB */
#define WCPU_1103_MPW2_PANIC_MEMDUMP_MAX_ADDR 0x60080000
/* 0x6007c400 */
#define WCPU_1103_MPW2_PANIC_MEMDUMP_STORE_ADDR \
    (WCPU_1103_MPW2_PANIC_MEMDUMP_MAX_ADDR - WCPU_1103_MPW2_PANIC_MEMDUMP_MAX_SIZE)

#define WCPU_1103_MPW2_ITCM_RAM_MAX_SIZE 0x88000 /* 544KB */
#define WCPU_1103_MPW2_ITCM_RAM_MAX_ADDR 0x8c000
/* 0x4000 */
#define WCPU_1103_MPW2_ITCM_RAM_STORE_ADDR (WCPU_1103_MPW2_ITCM_RAM_MAX_ADDR - WCPU_1103_MPW2_ITCM_RAM_MAX_SIZE)

#define WCPU_1103_MPW2_DTCM_RAM_MAX_SIZE 0x68000 /* 416KB */
#define WCPU_1103_MPW2_DTCM_RAM_MAX_ADDR 0x20068000
/* 0x20000000 */
#define WCPU_1103_MPW2_DTCM_RAM_STORE_ADDR (WCPU_1103_MPW2_DTCM_RAM_MAX_ADDR - WCPU_1103_MPW2_DTCM_RAM_MAX_SIZE)

#define WCPU_1103_MPW2_PKT_MEM_MAX_SIZE 0x90000 /* 576KB */
#define WCPU_1103_MPW2_PKT_MEM_MAX_ADDR 0x60090000
/* 0x60000000 */
#define WCPU_1103_MPW2_PKT_MEM_STORE_ADDR (WCPU_1103_MPW2_PKT_MEM_MAX_ADDR - WCPU_1103_MPW2_PKT_MEM_MAX_SIZE)
OAL_STATIC struct st_wifi_dump_mem_info g_hi1103_mpw2_meminfo[] = {
    {
        .file_name = "wifi_device_panic_mem",
        .mem_addr = WCPU_1103_MPW2_PANIC_MEMDUMP_STORE_ADDR,
        .size = WCPU_1103_MPW2_PANIC_MEMDUMP_MAX_SIZE, /* 15KB */
    },
    {
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = WCPU_1103_MPW2_ITCM_RAM_STORE_ADDR,
        .size = WCPU_1103_MPW2_ITCM_RAM_MAX_SIZE, /* 544KB */
    },
    {
        .file_name = "wifi_device_dtcm_ram",
        .mem_addr = WCPU_1103_MPW2_DTCM_RAM_STORE_ADDR,
        .size = WCPU_1103_MPW2_DTCM_RAM_MAX_SIZE, /* 416KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = WCPU_1103_MPW2_PKT_MEM_STORE_ADDR,
        .size = WCPU_1103_MPW2_PKT_MEM_MAX_SIZE, /* 576KB */
    },

};

/* pilot config **/
#define WCPU_1103_PILOT_PANIC_MEMDUMP_MAX_SIZE 0x3c00 /* 15KB */
#define WCPU_1103_PILOT_PANIC_MEMDUMP_MAX_ADDR 0x60080000
/* 0x6007c400 */
#define WCPU_1103_PILOT_PANIC_MEMDUMP_STORE_ADDR \
    (WCPU_1103_PILOT_PANIC_MEMDUMP_MAX_ADDR - WCPU_1103_PILOT_PANIC_MEMDUMP_MAX_SIZE)

#define WCPU_1103_PILOT_TICM_RAM_MAX_SIZE 0x98000 /* 608KB */
#define WCPU_1103_PILOT_TICM_RAM_MAX_ADDR 0xa8000
/* 0x10000 */
#define WCPU_1103_PILOT_TICM_RAM_STORE_ADDR (WCPU_1103_PILOT_TICM_RAM_MAX_ADDR - WCPU_1103_PILOT_TICM_RAM_MAX_SIZE)

#define WCPU_1103_PILOT_DTCM_RAM_MAX_SIZE 0x68000 /* 416KB */
#define WCPU_1103_PILOT_DTCM_RAM_MAX_ADDR 0x20080000
/* 0x20018000 */
#define WCPU_1103_PILOT_DTCM_RAM_STORE_ADDR (WCPU_1103_PILOT_DTCM_RAM_MAX_ADDR - WCPU_1103_PILOT_DTCM_RAM_MAX_SIZE)

#define WCPU_1103_PILOT_PKT_MEM_MAX_SIZE 0x80000 /* 512KB */
#define WCPU_1103_PILOT_PKT_MEM_MAX_ADDR 0x60080000
/* 0x60000000 */
#define WCPU_1103_PILOT_PKT_MEM_STORE_ADDR (WCPU_1103_PILOT_PKT_MEM_MAX_ADDR - WCPU_1103_PILOT_PKT_MEM_MAX_SIZE)
OAL_STATIC struct st_wifi_dump_mem_info g_hi1103_pilot_meminfo[] = {
    {
        .file_name = "wifi_device_panic_mem",
        .mem_addr = WCPU_1103_PILOT_PANIC_MEMDUMP_STORE_ADDR,
        .size = WCPU_1103_PILOT_PANIC_MEMDUMP_MAX_SIZE, /* 15KB */
    },
    {
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = WCPU_1103_PILOT_TICM_RAM_STORE_ADDR,
        .size = WCPU_1103_PILOT_TICM_RAM_MAX_SIZE, /* 608KB */
    },
    {
        .file_name = "wifi_device_dtcm_ram",
        .mem_addr = WCPU_1103_PILOT_DTCM_RAM_STORE_ADDR,
        .size = WCPU_1103_PILOT_DTCM_RAM_MAX_SIZE, /* 416KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = WCPU_1103_PILOT_PKT_MEM_STORE_ADDR,
        .size = WCPU_1103_PILOT_PKT_MEM_MAX_SIZE, /* 512KB */
    },

};

/* for 1105 */
#define WCPU_1105_ASIC_PANIC_MEMDUMP_MAX_SIZE 0x3c00 /* 15KB */
#define WCPU_1105_ASIC_PANIC_MEMDUMP_MAX_ADDR 0x60080000
/* 0x6007c400 */
#define WCPU_1105_ASIC_PANIC_MEMDUMP_STORE_ADDR \
    (WCPU_1105_ASIC_PANIC_MEMDUMP_MAX_ADDR - WCPU_1105_ASIC_PANIC_MEMDUMP_MAX_SIZE)

#define WCPU_1105_ASIC_ITCM_RAM_MAX_SIZE 0xb0000 /* 640+64KB */
#define WCPU_1105_ASIC_ITCM_RAM_MAX_ADDR 0xc0000
/* 0x10000 */
#define WCPU_1105_ASIC_ITCM_RAM_STORE_ADDR (WCPU_1105_ASIC_ITCM_RAM_MAX_ADDR - WCPU_1105_ASIC_ITCM_RAM_MAX_SIZE)

#define WCPU_1105_ASIC_DTCM_RAM_MAX_SIZE 0x68000 /* 416KB */
#define WCPU_1105_ASIC_DTCM_RAM_MAX_ADDR 0x20080000
/* 0x20018000 */
#define WCPU_1105_ASIC_DTCM_RAM_STORE_ADDR (WCPU_1105_ASIC_DTCM_RAM_MAX_ADDR - WCPU_1105_ASIC_DTCM_RAM_MAX_SIZE)

#define WCPU_1105_ASIC_PKT_MEM_MAX_SIZE 0x100000 /* 1024KB */
#define WCPU_1105_ASIC_PKT_MEM_MAX_ADDR 0x60100000
/* 0x60000000 */
#define WCPU_1105_ASIC_PKT_MEM_STORE_ADDR (WCPU_1105_ASIC_PKT_MEM_MAX_ADDR - WCPU_1105_ASIC_PKT_MEM_MAX_SIZE)
OAL_STATIC struct st_wifi_dump_mem_info g_hi1105_pilot_asic_meminfo[] = {
    {
        .file_name = "wifi_device_panic_mem",
        .mem_addr = WCPU_1105_ASIC_PANIC_MEMDUMP_STORE_ADDR,
        .size = WCPU_1105_ASIC_PANIC_MEMDUMP_MAX_SIZE, /* 15KB */
    },
    {
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = WCPU_1105_ASIC_ITCM_RAM_STORE_ADDR,
        .size = WCPU_1105_ASIC_ITCM_RAM_MAX_SIZE, /* 640+64KB */
    },
    {
        .file_name = "wifi_device_dtcm_ram",
        .mem_addr = WCPU_1105_ASIC_DTCM_RAM_STORE_ADDR,
        .size = WCPU_1105_ASIC_DTCM_RAM_MAX_SIZE, /* 416KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = WCPU_1105_ASIC_PKT_MEM_STORE_ADDR,
        .size = WCPU_1105_ASIC_PKT_MEM_MAX_SIZE, /* 512*2KB */
    },
};

#define WCPU_1105_FPGA_PANIC_MEMDUMP_MAX_SIZE 0x3c00 /* 15KB */
#define WCPU_1105_FPGA_PANIC_MEMDUMP_MAX_ADDR 0x60080000
/* 0x6007c400 */
#define WCPU_1105_FPGA_PANIC_MEMDUMP_STORE_ADDR \
    (WCPU_1105_FPGA_PANIC_MEMDUMP_MAX_ADDR - WCPU_1105_FPGA_PANIC_MEMDUMP_MAX_SIZE)

#define WCPU_1105_FPGA_TICM_RAM_MAX_SIZE 0xa0000 /* 640KB */
#define WCPU_1105_FPGA_TICM_RAM_MAX_ADDR 0xb0000
/* 0x10000 */
#define WCPU_1105_FPGA_TICM_RAM_STORE_ADDR (WCPU_1105_FPGA_TICM_RAM_MAX_ADDR - WCPU_1105_FPGA_TICM_RAM_MAX_SIZE)

#define WCPU_1105_FPGA_DTCM_RAM_MAX_SIZE 0x68000 /* 416KB */
#define WCPU_1105_FPGA_DTCM_RAM_MAX_ADDR 0x20080000
/* 0x20018000 */
#define WCPU_1105_FPGA_DTCM_RAM_STORE_ADDR (WCPU_1105_FPGA_DTCM_RAM_MAX_ADDR - WCPU_1105_FPGA_DTCM_RAM_MAX_SIZE)

#define WCPU_1105_FPGA_PKT_MEM_MAX_SIZE 0x80000 /* 512KB */
#define WCPU_1105_FPGA_PKT_MEM_MAX_ADDR 0x60080000
/* 0x60000000 */
#define WCPU_1105_FPGA_PKT_MEM_STORE_ADDR (WCPU_1105_FPGA_PKT_MEM_MAX_ADDR - WCPU_1105_FPGA_PKT_MEM_MAX_SIZE)
OAL_STATIC struct st_wifi_dump_mem_info g_hi1105_pilot_fpga_meminfo[] = {
    {
        .file_name = "wifi_device_panic_mem",
        .mem_addr = WCPU_1105_FPGA_PANIC_MEMDUMP_STORE_ADDR,
        .size = WCPU_1105_FPGA_PANIC_MEMDUMP_MAX_SIZE, /* 15KB */
    },
    {
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = WCPU_1105_FPGA_TICM_RAM_STORE_ADDR,
        .size = WCPU_1105_FPGA_TICM_RAM_MAX_SIZE, /* 640KB */
    },
    {
        .file_name = "wifi_device_dtcm_ram",
        .mem_addr = WCPU_1105_FPGA_DTCM_RAM_STORE_ADDR,
        .size = WCPU_1105_FPGA_DTCM_RAM_MAX_SIZE, /* 416KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = WCPU_1105_FPGA_PKT_MEM_STORE_ADDR,
        .size = WCPU_1105_FPGA_PKT_MEM_MAX_SIZE, /* 512KB */
    },
};

/* for 1106 */
#define HI1106_WL_WRAM_BASEADDR         0x40000
#define HI1106_WL_WRAM_LEN              0x170000

#define HI1106_WL_S6_SHARE_RAM_BASEADDR 0x2000000
#define HI1106_WL_SHARE_RAM_LEN         0xA2000

OAL_STATIC struct st_wifi_dump_mem_info g_hi1106_pilot_fpga_meminfo[] = {
    {
        /* Hi1106 对应WSRAM,名字需要修改对应oam_hisi */
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = HI1106_WL_WRAM_BASEADDR,
        .size = HI1106_WL_WRAM_LEN, /* 1472KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = HI1106_WL_S6_SHARE_RAM_BASEADDR,
        .size = HI1106_WL_SHARE_RAM_LEN, /* 648KB */
    },
};

OAL_STATIC struct st_wifi_dump_mem_info g_hi1106_pilot_asic_meminfo[] = {
    {
        /* Hi1106 对应WSRAM,名字需要修改对应oam_hisi */
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = HI1106_WL_WRAM_BASEADDR,
        .size = HI1106_WL_WRAM_LEN, /* 1472KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = HI1106_WL_S6_SHARE_RAM_BASEADDR,
        .size = HI1106_WL_SHARE_RAM_LEN, /* 648KB */
    },
};

#endif /* end of oal_hcc_bus_exception.h */
