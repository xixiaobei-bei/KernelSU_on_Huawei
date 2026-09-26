

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define HI11XX_LOG_MODULE_NAME "[PCIE_H]"
#define HISI_LOG_TAG           "[PCIE]"
#include "pcie_host.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include "pcie_chip.h"
#include "oal_thread.h"
#include "oam_ext_if.h"
#include "pcie_linux.h"
#include "pcie_reg.h"
#include "oal_hcc_host_if.h"
#include "oal_kernel_file.h"
#include "plat_firmware.h"
#include "plat_pm_wlan.h"
#include "board.h"
#include "securec.h"
#include "plat_pm.h"

#ifdef _PRE_WLAN_PKT_TIME_STAT
#include <hwnet/ipv4/wifi_delayst.h>
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_PCIE_HOST_C

OAL_STATIC oal_pcie_res *g_pci_res = NULL;

OAL_STATIC oal_kobject *g_conn_syfs_pci_object = NULL;

char *g_pcie_link_state_str[PCI_WLAN_LINK_BUTT + 1] = {
    [PCI_WLAN_LINK_DOWN] = "linkdown",
    [PCI_WLAN_LINK_DEEPSLEEP] = "deepsleep",
    [PCI_WLAN_LINK_UP] = "linkup",
    [PCI_WLAN_LINK_MEM_UP] = "memup",
    [PCI_WLAN_LINK_RES_UP] = "resup",
    [PCI_WLAN_LINK_WORK_UP] = "workup",
    [PCI_WLAN_LINK_BUTT] = "butt"
};

OAL_STATIC oal_pcie_bar_info g_en_bar_tab[] = {
    /*
     * 1103 4.7a 一个BAR [8MB]， 5.0a 为两个BAR[Bar0 8M  BAR1 16KB]
     * (因为1103 是64bit bar,所以对应bar index寄存器, 是对应bar index=2,
     *  参考 __pci_read_base 最后一行),
     * 第二个BAR 直接用MEM 方式 访问IATU表
     */
    {
        .bar_idx = OAL_PCI_BAR_0,
    },
};

int32_t g_hipcie_loglevel = PCI_LOG_INFO;
oal_debug_module_param(g_hipcie_loglevel, int, S_IRUGO | S_IWUSR);

/* soft fifo code had deleted 2020-8-21 master */
int32_t g_pcie_soft_fifo_enable = 0;
oal_debug_module_param(g_pcie_soft_fifo_enable, int, S_IRUGO | S_IWUSR);

int32_t g_pcie_ringbuf_bugfix_enable = 1;
oal_debug_module_param(g_pcie_ringbuf_bugfix_enable, int, S_IRUGO | S_IWUSR);

int32_t g_pcie_dma_data_check_enable = 0; /* Wi-Fi关闭时可以修改此标记 */
oal_debug_module_param(g_pcie_dma_data_check_enable, int, S_IRUGO | S_IWUSR);

int32_t g_pcie_dma_data_rx_hdr_init = 0; /* Wi-Fi关闭时可以修改此标记 */
oal_debug_module_param(g_pcie_dma_data_rx_hdr_init, int, S_IRUGO | S_IWUSR);

#ifdef CONFIG_ARCH_KIRIN_PCIE
int32_t g_pcie_dma_data_rx_check = 1; /* Wi-Fi关闭时可以修改此标记 */
#else
int32_t g_pcie_dma_data_rx_check = 0; /* Wi-Fi关闭时可以修改此标记 */
#endif
oal_debug_module_param(g_pcie_dma_data_rx_check, int, S_IRUGO | S_IWUSR);

/* 0 memcopy from kernel ,1 memcopy from self */
int32_t g_pcie_memcopy_type = 0;
oal_debug_module_param(g_pcie_memcopy_type, int, S_IRUGO | S_IWUSR);
EXPORT_SYMBOL_GPL(g_pcie_memcopy_type);

char *g_pci_loglevel_format[] = {
    KERN_ERR "[PCIE][ERR] ",
    KERN_WARNING "[PCIE][WARN]",
    KERN_INFO "[PCIE][INFO]",
    KERN_INFO "[PCIE][DBG] ",
};

/* Function Declare */
void oal_pcie_tx_netbuf_free(oal_pcie_res *pst_pci_res, oal_netbuf_stru *pst_netbuf);
int32_t oal_pcie_ringbuf_read_rd(oal_pcie_res *pst_pci_res, pcie_comm_ringbuf_type type);
int32_t oal_pcie_ringbuf_write(oal_pcie_res *pst_pci_res,
                               pcie_comm_ringbuf_type type, uint8_t *buf, uint32_t len);
uint32_t oal_pcie_comm_ringbuf_freecount(oal_pcie_res *pst_pci_res,
                                         pcie_comm_ringbuf_type type);

/* 函数定义 */
OAL_STATIC void oal_pcie_io_trans64_sub(uint64_t *dst, uint64_t *src, int32_t size)
{
    int32_t remain = size;

    forever_loop() {
        if (remain < 8) { /* 8bytes 最少传输字节数，剩余数据量不足时退出 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            rmb();
            wmb(); /* dsb */
#endif
            break;
        }
        if (remain >= 128) { /* 128 bytes, 根据剩余数据量拷贝，一次性拷贝128字节 */
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            remain -= 128; // 128 bytes
        } else if (remain >= 64) { /* 64bytes 根据剩余数据量拷贝，一次性拷贝64字节 */
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            remain -= 64; // 64 bytes
        } else if (remain >= 32) { /* 32 bytes 根据剩余数据量拷贝，一次性拷贝32字节 */
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            remain -= 32; // 32 bytes
        } else if (remain >= 16) { /* 16 bytes 根据剩余数据量拷贝，一次性拷贝16字节 */
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            remain -= 16; // 16 bytes
        } else {
            *((volatile uint64_t *)dst++) = *((volatile uint64_t *)src++);
            remain -= 8; /* 剩余数据量为8到15字节时，只拷贝8字节 */
        }
    }
}

void oal_pcie_io_trans32(uint32_t *dst, uint32_t *src, int32_t size)
{
    int32_t remain = size;

    forever_loop() {
        if (remain < 4) { /* 4 bytes 最少传输字节数，剩余数据量不足时退出 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            rmb();
            wmb(); /* dsb */
#endif
            break;
        }
        if (remain >= 64) { /* 64 bytes 根据剩余数据量拷贝，一次性拷贝64字节 */
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            remain -= 64; // 64 bytes
        } else if (remain >= 32) { /* 32 bytes 根据剩余数据量拷贝，一次性拷贝32字节 */
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            remain -= 32; // 32 bytes
        } else if (remain >= 16) { /* 16 bytes 根据剩余数据量拷贝，一次性拷贝16字节 */
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            remain -= 16; // 16 bytes
        } else if (remain >= 8) { /* 8 bytes 根据剩余数据量拷贝，一次性拷贝8字节 */
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            remain -= 8; // 8 bytes
        } else {
            *((volatile uint32_t *)dst++) = *((volatile uint32_t *)src++);
            remain -= 4; /* 4 bytes 剩余数据量为4到7字节时，只拷贝4字节 */
        }
    }
}

void oal_pcie_io_trans64(void *dst, void *src, int32_t size)
{
    uint32_t copy_size;
    int32_t remain = size;
    forever_loop() {
        if (remain < 4) { /* 4 bytes 最少传输字节数，剩余数据量不足时退出 */
            break;
        }

        if ((!((uintptr_t)src & 0x7)) && (remain >= 8)) { /* 8bytes */
            copy_size = OAL_ROUND_DOWN((uint32_t)remain, 8); /* 清除低3bit，保证8字节对齐 */
            remain -= copy_size;
            oal_pcie_io_trans64_sub(dst, src, (int32_t)copy_size);
            src += copy_size;
            dst += copy_size;
        } else if ((!((uintptr_t)src & 0x3)) && (remain >= 4)) { /* 4bytes */
            remain -= 4;                                         /* 拷贝4字节，则长度减少4 */
            *((volatile uint32_t *)dst) = *((volatile uint32_t *)src);
            /* 每次偏移4字节 */
            dst += sizeof(uint32_t);
            src += sizeof(uint32_t);
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "invalid argument, dst:%pK , src:%pK, remain:%d", dst, src, remain);
        }
    }
}

int32_t oal_pcie_memport_copy_test(void)
{
    unsigned long burst_size = 4096; // 4096 one mem page
    unsigned long timeout;
    unsigned long total_size;
    declare_time_cost_stru(cost);
    void *buff_src = NULL;
    void *buff_dst = NULL;
    uint64_t trans_size, us_to_s;

    buff_src = oal_memalloc(burst_size);
    if (buff_src == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc %lu buff failed", burst_size);
        return -OAL_EFAIL;
    }

    buff_dst = oal_memalloc(burst_size);
    if (buff_dst == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc %lu buff failed", burst_size);
        oal_free(buff_src);
        return -OAL_EFAIL;
    }

    oal_get_time_cost_start(cost);
    timeout = jiffies + oal_msecs_to_jiffies(2000); // 2000ms timeout
    total_size = 0;
    forever_loop() {
        if (oal_time_after(jiffies, timeout)) {
            break;
        }

        oal_pcie_io_trans((uintptr_t)buff_dst, (uintptr_t)buff_src, burst_size);
        total_size += burst_size;
    }

    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);

    us_to_s = time_cost_var_sub(cost);
    trans_size = ((total_size * 1000u * 1000u) >> PCIE_TRANS_US_OFFSET_BITS); // 1000 calc
    trans_size = div_u64(trans_size, us_to_s);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "memcopy: %llu Mbps, trans_time:%llu us, tran_size:%lu",
                         trans_size, us_to_s, total_size);

    oal_free(buff_src);
    oal_free(buff_dst);
    return 0;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE_PERFORMANCE
OAL_STATIC ktime_t g_mips_arry_total[PCIE_MIPS_BUTT];
OAL_STATIC ktime_t g_mips_arry[PCIE_MIPS_BUTT];
void oal_pcie_mips_start(pcie_mips_type type)
{
    if (oal_warn_on(type >= PCIE_MIPS_BUTT)) {
        oal_io_print("%s error: type:%d >= limit :%d", __FUNCTION__, type, PCIE_MIPS_BUTT);
        return;
    }
    g_mips_arry[type] = ktime_get();
}

void oal_pcie_mips_end(pcie_mips_type type)
{
    ktime_t end = ktime_get();

    if (oal_warn_on(type >= PCIE_MIPS_BUTT)) {
        oal_io_print("%s error: type:%d >= limit :%d", __FUNCTION__, type, PCIE_MIPS_BUTT);
        return;
    }

    g_mips_arry_total[type] = ktime_add(g_mips_arry_total[type], ktime_sub(end, g_mips_arry[type]));
}

void oal_pcie_mips_clear(void)
{
    memset_s(g_mips_arry, sizeof(g_mips_arry), 0, sizeof(g_mips_arry));
    memset_s(g_mips_arry_total, sizeof(g_mips_arry_total), 0, sizeof(g_mips_arry_total));
}

void oal_pcie_mips_show(void)
{
    int32_t i;
    int64_t trans_us, total_us;
    total_us = 0;
    for (i = 0; i < PCIE_MIPS_BUTT; i++) {
        trans_us = (uint64_t)ktime_to_us(g_mips_arry_total[i]);
        total_us += trans_us;
        oal_io_print("mips type:%d , trans_us :%llu us\n", i, trans_us);
    }
    oal_io_print("total_us :%llu us \n", total_us);
}

#endif
oal_pcie_res *oal_get_default_pcie_handler(void)
{
    return g_pci_res;
}

void oal_pcie_set_dma_data_rx_check_flag(int32_t flag)
{
    pci_print_log(PCI_LOG_INFO, "set dma rx flag from %d to %d", g_pcie_dma_data_rx_check, flag);
    g_pcie_dma_data_rx_check = flag;
}

/* 函数定义 */
int32_t oal_pcie_disable_regions(oal_pcie_res *pst_pci_res)
{
    if (oal_warn_on(pst_pci_res == NULL)) {
        return -OAL_ENODEV;
    }

    pst_pci_res->regions.inited = 0;
    pci_print_log(PCI_LOG_DBG, "disable_regions");
    return OAL_SUCC;
}

int32_t oal_pcie_enable_regions(oal_pcie_res *pst_pci_res)
{
    if (oal_warn_on(pst_pci_res == NULL)) {
        return -OAL_ENODEV;
    }

    pst_pci_res->regions.inited = 1;

    pci_print_log(PCI_LOG_DBG, "enable_regions");
    return OAL_SUCC;
}

OAL_STATIC void oal_pcie_iatu_reg_dump_by_viewport(oal_pcie_res *pci_res)
{
    int32_t index;
    int32_t ret;
    uint32_t reg = 0;
    uint32_t region_num;
    iatu_viewport_off vp;
    oal_pcie_region *region_base;
    oal_pci_dev_stru *pci_dev = pcie_res_to_dev(pci_res);

    region_num = pci_res->regions.region_nums;
    region_base = pci_res->regions.pst_regions;
    for (index = 0; index < region_num; index++, region_base++) {
        vp.bits.region_dir = HI_PCI_IATU_INBOUND;
        vp.bits.region_index = index; /* iatu index */
        ret = oal_pci_write_config_dword(pci_dev, HI_PCI_IATU_VIEWPORT_OFF, vp.as_dword);
        if (ret) {
            pci_print_log(PCI_LOG_ERR, "dump write [0x%8x:0x%8x] pcie failed, ret=%d\n",
                          HI_PCI_IATU_VIEWPORT_OFF, vp.as_dword, ret);
            break;
        }

        ret = oal_pci_read_config_dword(pci_dev, HI_PCI_IATU_VIEWPORT_OFF, &reg);
        if (ret) {
            pci_print_log(PCI_LOG_ERR, "dump read [0x%8x] pcie failed, ret=%d\n", HI_PCI_IATU_VIEWPORT_OFF, ret);
            break;
        }

        pci_print_log(PCI_LOG_INFO, "INBOUND iatu index:%d 's register:\n", index);

        if (reg != vp.as_dword) {
            pci_print_log(PCI_LOG_ERR, "dump write [0x%8x:0x%8x] pcie viewport failed value still 0x%8x\n",
                          HI_PCI_IATU_VIEWPORT_OFF, vp.as_dword, reg);
            break;
        }

        print_pcie_config_reg(pci_dev, HI_PCI_IATU_VIEWPORT_OFF);

        print_pcie_config_reg(pci_dev, hi_pci_iatu_region_ctrl_1_off_inbound_i(HI_PCI_IATU_BOUND_BASE_OFF));

        print_pcie_config_reg(pci_dev, hi_pci_iatu_region_ctrl_2_off_inbound_i(HI_PCI_IATU_BOUND_BASE_OFF));

        print_pcie_config_reg(pci_dev, hi_pci_iatu_lwr_base_addr_off_inbound_i(HI_PCI_IATU_BOUND_BASE_OFF));

        print_pcie_config_reg(pci_dev, hi_pci_iatu_upper_base_addr_off_inbound_i(HI_PCI_IATU_BOUND_BASE_OFF));

        print_pcie_config_reg(pci_dev, hi_pci_iatu_limit_addr_off_inbound_i(HI_PCI_IATU_BOUND_BASE_OFF));

        print_pcie_config_reg(pci_dev, hi_pci_iatu_lwr_target_addr_off_inbound_i(HI_PCI_IATU_BOUND_BASE_OFF));

        print_pcie_config_reg(pci_dev, hi_pci_iatu_upper_target_addr_off_inbound_i(HI_PCI_IATU_BOUND_BASE_OFF));
    }
}

OAL_STATIC void oal_pcie_iatu_reg_dump_by_membar(oal_pcie_res *pci_res)
{
    void *inbound_addr = NULL;

    int32_t index;
    uint32_t region_num;
    oal_pcie_region *region_base = NULL;

    if (pci_res->st_iatu_bar.st_region.vaddr == NULL) {
        pci_print_log(PCI_LOG_ERR, "iatu bar1 vaddr is null");
        return;
    }

    inbound_addr = pci_res->st_iatu_bar.st_region.vaddr;

    region_num = pci_res->regions.region_nums;
    region_base = pci_res->regions.pst_regions;
    for (index = 0; index < region_num; index++, region_base++) {
        if (index >= 16) { /* dump size为0x2000，一次偏移0x200，这里16代表dump了所有空间 */
            break;
        }

        pci_print_log(PCI_LOG_INFO, "INBOUND iatu index:%d 's register:\n", index);

        oal_pcie_print_config_reg_bar(pci_res,
                                      hi_pci_iatu_region_ctrl_1_off_inbound_i(hi_pci_iatu_inbound_base_off(index)),
                                      "hi_pci_iatu_region_ctrl_1_off_inbound_i");

        oal_pcie_print_config_reg_bar(pci_res,
                                      hi_pci_iatu_region_ctrl_2_off_inbound_i(hi_pci_iatu_inbound_base_off(index)),
                                      "hi_pci_iatu_region_ctrl_2_off_inbound_i");

        oal_pcie_print_config_reg_bar(pci_res,
                                      hi_pci_iatu_lwr_base_addr_off_inbound_i(hi_pci_iatu_inbound_base_off(index)),
                                      "hi_pci_iatu_lwr_base_addr_off_inbound_i");

        oal_pcie_print_config_reg_bar(pci_res,
                                      hi_pci_iatu_upper_base_addr_off_inbound_i(hi_pci_iatu_inbound_base_off(index)),
                                      "hi_pci_iatu_upper_base_addr_off_inbound_i");

        oal_pcie_print_config_reg_bar(pci_res,
                                      hi_pci_iatu_limit_addr_off_inbound_i(hi_pci_iatu_inbound_base_off(index)),
                                      "hi_pci_iatu_limit_addr_off_inbound_i");

        oal_pcie_print_config_reg_bar(pci_res,
                                      hi_pci_iatu_lwr_target_addr_off_inbound_i(hi_pci_iatu_inbound_base_off(index)),
                                      "hi_pci_iatu_lwr_target_addr_off_inbound_i");

        oal_pcie_print_config_reg_bar(pci_res,
                                      hi_pci_iatu_upper_target_addr_off_inbound_i(hi_pci_iatu_inbound_base_off(index)),
                                      "hi_pci_iatu_upper_target_addr_off_inbound_i");
    }
}

/* bar and iATU table config */
/* set ep inbound, host->device */
OAL_STATIC void oal_pcie_iatu_reg_dump(oal_pcie_res *pci_res)
{
    if (pci_res->chip_info.membar_support == OAL_FALSE) {
        oal_pcie_iatu_reg_dump_by_viewport(pci_res);
    } else {
        oal_pcie_iatu_reg_dump_by_membar(pci_res);
    }
}

void oal_pcie_regions_info_dump(oal_pcie_res *pci_res)
{
    int32_t index;
    uint32_t region_num;
    oal_pcie_region *region_base;

    region_num = pci_res->regions.region_nums;
    region_base = pci_res->regions.pst_regions;

    if (oal_warn_on(!pci_res->regions.inited)) {
        return;
    }

    if (region_num) {
        oal_io_print("regions[%u] info dump\n", region_num);
    }

    for (index = 0; index < region_num; index++, region_base++) {
        oal_io_print("[%15s]va:0x%p, pa:0x%llx, [pci start:0x%8llx end:0x%8llx],[cpu start:0x%8llx end:0x%8llx],\
                     size:%u, flag:0x%x\n",
                     region_base->name,
                     region_base->vaddr,
                     region_base->paddr,
                     region_base->pci_start,
                     region_base->pci_end,
                     region_base->cpu_start,
                     region_base->cpu_end,
                     region_base->size,
                     region_base->flag);
    }
}

OAL_STATIC int32_t oal_pcie_set_inbound_by_viewport(oal_pcie_res *pci_res)
{
    uint32_t reg = 0;
    uint32_t ret = OAL_SUCC;
    edma_paddr_t start, target, end;
    iatu_viewport_off vp;
    iatu_region_ctrl_2_off ctr2;
    int32_t index;
    uint32_t region_num;
    oal_pcie_region *region_base;
    oal_pci_dev_stru *pci_dev = pcie_res_to_dev(pci_res);
    region_num = pci_res->regions.region_nums;
    region_base = pci_res->regions.pst_regions;

    for (index = 0; index < region_num; index++, region_base++) {
        vp.bits.region_dir = HI_PCI_IATU_INBOUND;
        vp.bits.region_index = index; /* iatu index */
        ret = oal_pci_write_config_dword(pci_dev, HI_PCI_IATU_VIEWPORT_OFF, vp.as_dword);
        if (ret) {
            pci_print_log(PCI_LOG_ERR, "write [0x%8x:0x%8x] pcie failed, ret=%u\n",
                          HI_PCI_IATU_VIEWPORT_OFF, vp.as_dword, ret);
            return -OAL_EIO;
        }

        /* 是否需要回读等待 */
        ret = oal_pci_read_config_dword(pci_dev, HI_PCI_IATU_VIEWPORT_OFF, &reg);
        if (ret) {
            pci_print_log(PCI_LOG_ERR, "read [0x%8x] pcie failed, index:%d, ret=%u\n",
                          HI_PCI_IATU_VIEWPORT_OFF, index, ret);
            return -OAL_EIO;
        }

        if (reg != vp.as_dword) {
            /* 1.viewport 没有切换完成 2. iatu配置个数超过了Soc的最大个数 */
            pci_print_log(PCI_LOG_ERR,
                          "write [0x%8x:0x%8x] pcie viewport failed value still 0x%8x, region's index:%d\n",
                          HI_PCI_IATU_VIEWPORT_OFF, vp.as_dword, reg, index);
            return -OAL_EIO;
        }

        ret |= (uint32_t)oal_pci_write_config_dword(pci_dev,
                                                    hi_pci_iatu_region_ctrl_1_off_inbound_i(
                                                        HI_PCI_IATU_BOUND_BASE_OFF),
                                                    0x0);

        ctr2.as_dword = 0;
        ctr2.bits.region_en = 1;
        ctr2.bits.bar_num = region_base->bar_info->bar_idx;
        ret |= (uint32_t)oal_pci_write_config_dword(pci_dev,
                                                    hi_pci_iatu_region_ctrl_2_off_inbound_i(
                                                        HI_PCI_IATU_BOUND_BASE_OFF),
                                                    ctr2.as_dword);

        /* Host侧64位地址的低32位地址 */
        start.addr = region_base->bus_addr;
        pci_print_log(PCI_LOG_INFO, "PCIe inbound bus addr:0x%llx", start.addr);
        ret |= (uint32_t)oal_pci_write_config_dword(pci_dev,
                                                    hi_pci_iatu_lwr_base_addr_off_inbound_i(
                                                        HI_PCI_IATU_BOUND_BASE_OFF),
                                                    start.bits.low_addr);
        ret |= (uint32_t)oal_pci_write_config_dword(pci_dev,
                                                    hi_pci_iatu_upper_base_addr_off_inbound_i(
                                                        HI_PCI_IATU_BOUND_BASE_OFF),
                                                    start.bits.high_addr);

        end.addr = start.addr + region_base->size - 1;
        if (start.bits.high_addr != end.bits.high_addr) {
            /* 如果跨了4G地址应该多配置一个iatu表项，待增加 */
            pci_print_log(PCI_LOG_ERR, "iatu high 32 bits must same![start:0x%llx, end:0x%llx]", start.addr, end.addr);
            return -OAL_EIO;
        }
        ret |= (uint32_t)oal_pci_write_config_dword(pci_dev,
                                                    hi_pci_iatu_limit_addr_off_inbound_i(HI_PCI_IATU_BOUND_BASE_OFF),
                                                    end.bits.low_addr);

        /* Device侧对应的地址(PCI看到的地址) */
        target.addr = region_base->pci_start;
        ret |= (uint32_t)oal_pci_write_config_dword(pci_dev,
                                                    hi_pci_iatu_lwr_target_addr_off_inbound_i(
                                                        HI_PCI_IATU_BOUND_BASE_OFF), target.bits.low_addr);
        ret |= (uint32_t)oal_pci_write_config_dword(pci_dev,
                                                    hi_pci_iatu_upper_target_addr_off_inbound_i(
                                                        HI_PCI_IATU_BOUND_BASE_OFF),
                                                    target.bits.high_addr);
    }

    /* 配置命令寄存器                                                                         */
    /* BIT0 = 1(I/O Space Enable), BIT1 = 1(Memory Space Enable), BIT2 = 1(Bus Master Enable) */
    ret |= (uint32_t)oal_pci_write_config_word(pci_dev, 0x04, 0x7);
    if (ret) {
        pci_print_log(PCI_LOG_ERR, "pci write iatu config failed ret=%d\n", ret);
        return -OAL_EIO;
    }

    if (pci_dbg_condtion()) {
        oal_pcie_iatu_reg_dump(pci_res);
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_set_inbound_by_membar(oal_pcie_res *pci_res)
{
    void *inbound_addr = NULL;

    int32_t ret;
    edma_paddr_t start, target, end;
    iatu_region_ctrl_2_off ctr2;
    int32_t index;
    uint32_t region_num;
    oal_pcie_region *region_base;
    oal_pci_dev_stru *pci_dev = pcie_res_to_dev(pci_res);
    region_num = pci_res->regions.region_nums;
    region_base = pci_res->regions.pst_regions;

    if (pci_res->st_iatu_bar.st_region.vaddr == NULL) {
        pci_print_log(PCI_LOG_ERR, "iatu bar1 vaddr is null");
        return -OAL_ENOMEM;
    }

    inbound_addr = pci_res->st_iatu_bar.st_region.vaddr;

    for (index = 0; index < region_num; index++, region_base++) {
        if (index >= 16) { /* 设置的大小为0x2000，一次偏移0x200，这里16代表设置了所有空间 */
            pci_print_log(PCI_LOG_ERR, "iatu regions too many, start:0x%llx", region_base->bar_info->start);
            break;
        }

        oal_writel(0x0, inbound_addr + hi_pci_iatu_region_ctrl_1_off_inbound_i(hi_pci_iatu_inbound_base_off(index)));

        ctr2.as_dword = 0;
        ctr2.bits.region_en = 1;
        ctr2.bits.bar_num = region_base->bar_info->bar_idx;
        oal_writel(ctr2.as_dword,
                   inbound_addr + hi_pci_iatu_region_ctrl_2_off_inbound_i(hi_pci_iatu_inbound_base_off(index)));

        /* Host侧64位地址的低32位地址 */
        start.addr = region_base->bus_addr;
        pci_print_log(PCI_LOG_INFO, "PCIe inbound bus addr:0x%llx", start.addr);
        oal_writel(start.bits.low_addr,
                   inbound_addr + hi_pci_iatu_lwr_base_addr_off_inbound_i(hi_pci_iatu_inbound_base_off(index)));
        oal_writel(start.bits.high_addr,
                   inbound_addr + hi_pci_iatu_upper_base_addr_off_inbound_i(hi_pci_iatu_inbound_base_off(index)));

        end.addr = start.addr + region_base->size - 1;
        if (start.bits.high_addr != end.bits.high_addr) {
            /* 如果跨了4G地址应该多配置一个iatu表项，待增加 */
            pci_print_log(PCI_LOG_ERR, "iatu high 32 bits must same![start:0x%llx, end:0x%llx]", start.addr, end.addr);
            return -OAL_EIO;
        }
        oal_writel(end.bits.low_addr,
                   inbound_addr + hi_pci_iatu_limit_addr_off_inbound_i(hi_pci_iatu_inbound_base_off(index)));

        /* Device侧对应的地址(PCI看到的地址) */
        target.addr = region_base->pci_start;
        oal_writel(target.bits.low_addr,
                   inbound_addr + hi_pci_iatu_lwr_target_addr_off_inbound_i(hi_pci_iatu_inbound_base_off(index)));
        oal_writel(target.bits.high_addr,
                   inbound_addr + hi_pci_iatu_upper_target_addr_off_inbound_i(hi_pci_iatu_inbound_base_off(index)));
    }

    if (index) {
        /* 回读可以保证之前的IATU立刻生效 */
        uint32_t callback_read;
        callback_read = oal_readl(inbound_addr +
                                  hi_pci_iatu_region_ctrl_1_off_inbound_i(hi_pci_iatu_inbound_base_off(0)));
        oal_reference(callback_read);
    }

    /* 配置命令寄存器                                                                         */
    /* BIT0 = 1(I/O Space Enable), BIT1 = 1(Memory Space Enable), BIT2 = 1(Bus Master Enable) */
    ret = oal_pci_write_config_word(pci_dev, 0x04, 0x7);
    if (ret) {
        pci_print_log(PCI_LOG_ERR, "pci write iatu config failed ret=%d\n", ret);
        return -OAL_EIO;
    }

    if (pci_dbg_condtion()) {
        oal_pcie_iatu_reg_dump(pci_res);
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_set_inbound(oal_pcie_res *pst_pci_res)
{
    if (pst_pci_res->chip_info.membar_support == OAL_FALSE) {
        return oal_pcie_set_inbound_by_viewport(pst_pci_res);
    } else {
        return oal_pcie_set_inbound_by_membar(pst_pci_res);
    }
}

void oal_pcie_iatu_outbound_dump_by_membar(const void *outbound_addr, uint32_t index)
{
    if (outbound_addr == NULL) {
        pci_print_log(PCI_LOG_ERR, "iatu bar1 vaddr is null");
        return;
    }

    pci_print_log(PCI_LOG_INFO, "OUTBOUND iatu index:%d 's register:\n", index);
    pci_print_log(PCI_LOG_INFO, "%s : 0x%x", "hi_pci_iatu_region_ctrl_1_off_outbound_i",
                  oal_readl(outbound_addr
                  + hi_pci_iatu_region_ctrl_1_off_outbound_i(hi_pci_iatu_outbound_base_off(index))));
    pci_print_log(PCI_LOG_INFO, "%s : 0x%x", "hi_pci_iatu_region_ctrl_2_off_outbound_i",
                  oal_readl(outbound_addr + hi_pci_iatu_region_ctrl_2_off_outbound_i(
                      hi_pci_iatu_outbound_base_off(index))));
    pci_print_log(PCI_LOG_INFO, "%s : 0x%x", "hi_pci_iatu_lwr_base_addr_off_outbound_i",
                  hi_pci_iatu_lwr_base_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
    pci_print_log(PCI_LOG_INFO, "%s : 0x%x", "hi_pci_iatu_upper_base_addr_off_outbound_i",
                  hi_pci_iatu_upper_base_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
    pci_print_log(PCI_LOG_INFO, "%s : 0x%x", "hi_pci_iatu_limit_addr_off_outbound_i",
                  hi_pci_iatu_limit_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
    pci_print_log(PCI_LOG_INFO, "%s : 0x%x", "hi_pci_iatu_lwr_target_addr_off_outbound_i",
                  hi_pci_iatu_lwr_target_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
    pci_print_log(PCI_LOG_INFO, "%s : 0x%x", "hi_pci_iatu_upper_target_addr_off_outbound_i",
                  hi_pci_iatu_upper_target_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
}

int32_t oal_pcie_set_outbound_iatu_by_membar(void* pst_bar1_vaddr, uint32_t index, uint64_t src_addr,
                                             uint64_t dst_addr, uint64_t limit_size)
{
    /* IATU 对齐要求,开始结束地址按照4K对齐 */
    iatu_region_ctrl_1_off ctr1;
    iatu_region_ctrl_2_off ctr2;
    void *bound_addr = pst_bar1_vaddr;

    uint32_t src_addrl, src_addrh, value;
    uint32_t dst_addrl, dst_addrh;
    uint64_t limit_addr = src_addr + limit_size - 1;

    if (oal_warn_on(pst_bar1_vaddr == NULL)) {
        pci_print_log(PCI_LOG_ERR, "pst_bar1_vaddr is null");
        return -OAL_ENODEV;
    }

    src_addrl = (uint32_t)src_addr;
    src_addrh = (uint32_t)(src_addr >> 32); /* 32 high bits */
    dst_addrl = (uint32_t)dst_addr;
    dst_addrh = (uint32_t)(dst_addr >> 32); /* 32 high bits */

    ctr1.as_dword = 0;

    if (limit_addr >> 32) { // 32 hight bits
        ctr1.bits.inc_region_size = 1; /* more than 4G */
    }

    ctr2.as_dword = 0;
    ctr2.bits.region_en = 1;
    ctr2.bits.bar_num = 0x0;

    oal_writel(ctr1.as_dword, bound_addr +
               hi_pci_iatu_region_ctrl_1_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
    oal_writel(ctr2.as_dword, bound_addr +
               hi_pci_iatu_region_ctrl_2_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));

    oal_writel(src_addrl, bound_addr +
               hi_pci_iatu_lwr_base_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
    oal_writel(src_addrh, bound_addr +
               hi_pci_iatu_upper_base_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));

    oal_writel((uint32_t)limit_addr, bound_addr +
               hi_pci_iatu_limit_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
    // 32 hight bits
    oal_writel((uint32_t)(limit_addr >> 32), bound_addr +
               hi_pci_iatu_uppr_limit_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));

    oal_writel(dst_addrl, bound_addr +
               hi_pci_iatu_lwr_target_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
    oal_writel(dst_addrh,
               bound_addr + hi_pci_iatu_upper_target_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));

    value = oal_readl(bound_addr + hi_pci_iatu_lwr_base_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
    if (value != src_addrl) {
        pci_print_log(PCI_LOG_ERR,
                      "callback read 0x%x failed src_addr=0x%llx, dst_addr=0x%llx limit=0x%llx, index = %u",
                      value, src_addr, dst_addr,  limit_addr, index);
        oal_pcie_iatu_outbound_dump_by_membar(pst_bar1_vaddr, index);
        return -OAL_EFAIL;
    } else {
        pci_print_log(PCI_LOG_INFO, "outbound  src_addr=0x%llx, dst_addr=0x%llx limit=0x%llx, index = %u",
                      src_addr, dst_addr, limit_addr, index);
        return OAL_SUCC;
    }
}

int32_t oal_pcie_set_outbound_by_membar(oal_pcie_res *pst_pci_res)
{
    return oal_pcie_set_outbound_membar(pst_pci_res, &pst_pci_res->st_iatu_bar);
}

hcc_bus *oal_pcie_get_current_bus(void)
{
    uint32_t dev_id = HCC_CHIP_110X_DEV;
    hcc_bus *pst_bus = NULL;
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(dev_id);

    if (pst_bus_dev == NULL) {
        return NULL;
    }
    pst_bus = pst_bus_dev->cur_bus;

    if (hcc_bus_isvalid(pst_bus) != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "bus %d is not ready", HCC_BUS_PCIE);
        return NULL;
    }

    return pst_bus;
}

oal_bool_enum_uint8 oal_pcie_link_state_up(void)
{
    hcc_bus *hi_bus = oal_pcie_get_current_bus();
    oal_pcie_res *g_pci_res = NULL;
    oal_pcie_linux_res *pci_lres = NULL;

    if (hi_bus == NULL) {
        return OAL_FALSE;
    }

    pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    if (pci_lres == NULL) {
        return OAL_FALSE;
    }

    g_pci_res = pci_lres->pst_pci_res;
    if (g_pci_res == NULL) {
        return OAL_FALSE;
    }

    return g_pci_res->link_state == PCI_WLAN_LINK_WORK_UP;
}
oal_module_symbol(oal_pcie_link_state_up);

/* PCIE EP Master 口访问,主芯片CPU访问片内地址空间(注意不能访问片内PCIE Slave口空间) for WiFi TAE
 *    将devcpu看到的地址转换成host侧看到的物理地址和虚拟地址 */
int32_t oal_pcie_devca_to_hostva(uint32_t ul_chip_id, uint64_t dev_cpuaddr, uint64_t *host_va)
{
    oal_reference(ul_chip_id);
    return hcc_bus_master_address_switch(oal_pcie_get_current_bus(), dev_cpuaddr, host_va, MASTER_WCPUADDR_TO_VA);
}
oal_module_symbol(oal_pcie_devca_to_hostva);

/* 虚拟地址转换成WCPU看到的地址 */
int32_t oal_pcie_get_dev_ca(uint32_t ul_chip_id, void *host_va, uint64_t* dev_cpuaddr)
{
    oal_reference(ul_chip_id);
    return hcc_bus_master_address_switch(oal_pcie_get_current_bus(),
                                         (uint64_t)(uintptr_t)host_va, dev_cpuaddr, MASTER_VA_TO_WCPUADDR);
}
oal_module_symbol(oal_pcie_get_dev_ca);

/* PCIE EP Slave 口看到的地址转换 --片内外设通过PCIE访问主芯片DDR空间 */
/* 地址转换主芯片的DDR设备地址转换成device的SLAVE口地址
 *  注意这里的DDR设备地址(iova)不是的直接物理地址，而是对应Kernel的DMA地址
 *  设备地址不能通过phys_to_virt/virt_to_phys直接转换
 */
int32_t pcie_if_hostca_to_devva(uint32_t chip_id, uint64_t host_iova, uint64_t *addr)
{
    oal_reference(chip_id);
    return hcc_bus_slave_address_switch(oal_pcie_get_current_bus(), host_iova, addr, SLAVE_IOVA_TO_PCI_SLV);
}
EXPORT_SYMBOL_GPL(pcie_if_hostca_to_devva);

/* 地址转换主芯片的DDR设备地址转换成device的SLAVE口地址 */
int32_t pcie_if_devva_to_hostca(uint32_t chip_id, uint64_t devva, uint64_t *host_iova)
{
    oal_reference(chip_id);
    return hcc_bus_slave_address_switch(oal_pcie_get_current_bus(), devva, host_iova, SLAVE_PCI_SLV_TO_IOVA);
}
EXPORT_SYMBOL_GPL(pcie_if_devva_to_hostca);

/* set ep outbound, device->host */
OAL_STATIC int32_t oal_pcie_set_outbound(oal_pcie_res *pst_pci_res)
{
    if (pst_pci_res->chip_info.membar_support == OAL_TRUE) {
        return oal_pcie_set_outbound_by_membar(pst_pci_res);
    }
    return OAL_SUCC;
}

OAL_STATIC void oal_pcie_iatu_exit(oal_pcie_res *pst_pci_res)
{
}

int32_t oal_pcie_iatu_init(oal_pcie_res *pst_pci_res)
{
    int32_t ret;

    if (pst_pci_res == NULL) {
        return -OAL_ENODEV;
    }

    if (!pst_pci_res->regions.inited) {
        pci_print_log(PCI_LOG_ERR, "pcie regions is disabled, iatu config failed");
        return -OAL_EIO;
    }

    ret = oal_pcie_set_inbound(pst_pci_res);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "pcie inbound set failed ret=%d\n", ret);
        return ret;
    }

    ret = oal_pcie_set_outbound(pst_pci_res);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "pcie outbound set failed ret=%d\n", ret);
        return ret;
    }

    /* mem方式访问使能 */
    oal_pcie_change_link_state(pst_pci_res, PCI_WLAN_LINK_MEM_UP);
    return OAL_SUCC;
}

OAL_STATIC void oal_pcie_ctrl_base_address_exit(oal_pcie_res *pst_pci_res)
{
    pst_pci_res->pst_pci_dma_ctrl_base = NULL;
    pst_pci_res->pst_pci_ctrl_base = NULL;
    pst_pci_res->pst_pci_dbi_base = NULL;
    pst_pci_res->pst_ete_base = NULL;
}

OAL_STATIC int32_t oal_pcie_ctrl_base_address_init(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    pci_addr_map st_map;

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, pst_pci_res->chip_info.addr_info.pcie_ctrl, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "get dev address 0x%x failed", pst_pci_res->chip_info.addr_info.pcie_ctrl);
        oal_pcie_regions_info_dump(pst_pci_res);
        return -OAL_EFAIL;
    }
    pst_pci_res->pst_pci_ctrl_base = (void *)st_map.va;

    if (pst_pci_res->chip_info.edma_support == OAL_TRUE) {
        ret = oal_pcie_inbound_ca_to_va(pst_pci_res, pst_pci_res->chip_info.addr_info.edma_ctrl, &st_map);
        if (ret != OAL_SUCC) {
            pci_print_log(PCI_LOG_ERR, "get dev address 0x%x failed", pst_pci_res->chip_info.addr_info.edma_ctrl);
            oal_pcie_regions_info_dump(pst_pci_res);
            oal_pcie_ctrl_base_address_exit(pst_pci_res);
            return -OAL_EFAIL;
        }
        pst_pci_res->pst_pci_dma_ctrl_base = (void *)st_map.va;
    }

    /* dbi base */
    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, pst_pci_res->chip_info.addr_info.dbi, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "get dev address 0x%x failed", PCIE_CONFIG_BASE_ADDRESS);
        oal_pcie_regions_info_dump(pst_pci_res);
        oal_pcie_ctrl_base_address_exit(pst_pci_res);
        return -OAL_EFAIL;
    }
    pst_pci_res->pst_pci_dbi_base = (void *)st_map.va;

    if (pst_pci_res->chip_info.ete_support == OAL_TRUE) {
        /* ete base */
        ret = oal_pcie_inbound_ca_to_va(pst_pci_res, pst_pci_res->chip_info.addr_info.ete_ctrl, &st_map);
        if (ret != OAL_SUCC) {
            pci_print_log(PCI_LOG_ERR, "get dev address 0x%x failed", PCIE_CONFIG_BASE_ADDRESS);
            oal_pcie_regions_info_dump(pst_pci_res);
            oal_pcie_ctrl_base_address_exit(pst_pci_res);
            return -OAL_EFAIL;
        }
        pst_pci_res->pst_ete_base = (void *)st_map.va;
    }

    pci_print_log(PCI_LOG_DBG, "ctrl base addr init succ, pci va:0x%p, fifo va:0x%p, dbi va:0x%p, ete va:0x%p",
                  pst_pci_res->pst_pci_ctrl_base, pst_pci_res->pst_pci_dma_ctrl_base, pst_pci_res->pst_pci_dbi_base,
                  pst_pci_res->pst_ete_base);
    return OAL_SUCC;
}

OAL_STATIC void oal_pcie_regions_exit(oal_pcie_res *pst_pci_res)
{
    int32_t index;
    uint32_t region_num;
    oal_pcie_region *region_base = NULL;
    pci_print_log(PCI_LOG_INFO, "oal_pcie_regions_exit\n");
    pst_pci_res->regions.inited = 0;

    region_num = pst_pci_res->regions.region_nums;
    region_base = pst_pci_res->regions.pst_regions;

    /* 释放申请的地址空间 */
    for (index = 0; index < region_num; index++, region_base++) {
        if (region_base->vaddr != NULL) {
            oal_iounmap(region_base->vaddr);
            oal_release_mem_region(region_base->paddr, region_base->size);
            region_base->vaddr = NULL;
        }
    }
}

void oal_pcie_bar1_exit(oal_pcie_iatu_bar *pst_iatu_bar)
{
    if (pst_iatu_bar->st_region.vaddr == NULL) {
        return;
    }

    oal_iounmap(pst_iatu_bar->st_region.vaddr);
    oal_release_mem_region(pst_iatu_bar->st_region.paddr, pst_iatu_bar->st_region.size);
    pst_iatu_bar->st_region.vaddr = NULL;
}

OAL_STATIC void oal_pcie_iatu_bar_exit(oal_pcie_res *pst_pci_res)
{
    oal_pcie_bar1_exit(&pst_pci_res->st_iatu_bar);
}

int32_t oal_pcie_bar1_init(oal_pcie_iatu_bar* pst_iatu_bar)
{
    oal_resource *pst_res = NULL;
    oal_pcie_bar_info *bar_base = NULL;
    oal_pcie_region *region_base = NULL;

    if (pst_iatu_bar->st_bar_info.size == 0) {
        return OAL_SUCC;
    }

    bar_base = &pst_iatu_bar->st_bar_info;
    region_base = &pst_iatu_bar->st_region;

    /* Bar1 专门用于配置 iatu表 */
    region_base->vaddr = NULL;            /* remap 后的虚拟地址 */
    region_base->paddr = bar_base->start; /* Host CPU看到的物理地址 */
    region_base->bus_addr = 0x0;
    region_base->res = NULL;
    region_base->bar_info = bar_base;
    region_base->size = bar_base->size;
    region_base->name = "iatu_bar1";
    region_base->flag = OAL_IORESOURCE_REG;

    pst_res = oal_request_mem_region(region_base->paddr, region_base->size, region_base->name);
    if (pst_res == NULL) {
        goto failed_request_region;
    }

    /* remap */
    if (region_base->flag & OAL_IORESOURCE_REG) {
        /* 寄存器映射成非cache段, 不需要刷cache */
        region_base->vaddr = oal_ioremap_nocache(region_base->paddr, region_base->size);
    } else {
        /* cache 段，注意要刷cache */
        region_base->vaddr = oal_ioremap(region_base->paddr, region_base->size);
    }

    if (region_base->vaddr == NULL) {
        oal_release_mem_region(region_base->paddr, region_base->size);
        goto failed_remap;
    }

    /* remap and request succ. */
    region_base->res = pst_res;

    pci_print_log(PCI_LOG_INFO, "iatu bar1 virtual address:%p", region_base->vaddr);

    return OAL_SUCC;
failed_remap:
    oal_iounmap(region_base->vaddr);
    oal_release_mem_region(region_base->paddr, region_base->size);
    region_base->vaddr = NULL;
failed_request_region:
    return -OAL_ENOMEM;
}

OAL_STATIC int32_t oal_pcie_iatu_bar_init(oal_pcie_res *pst_pci_res)
{
    return oal_pcie_bar1_init(&pst_pci_res->st_iatu_bar);
}

OAL_STATIC int32_t oal_pcie_regions_init(oal_pcie_res *pst_pci_res)
{
    /* 初始化DEVICE 每个段分配的HOST物理地址，然后做remap */
    void *vaddr = NULL;
    int32_t index, region_idx, bar_used_size;
    uint32_t bar_num, region_num;
    oal_pcie_bar_info *bar_base = NULL;
    oal_pcie_region *region_base = NULL;
    oal_resource *pst_res = NULL;

    if (oal_warn_on(pst_pci_res->regions.inited)) {
        /* 不能重复初始化 */
        return -OAL_EBUSY;
    }

    bar_num = pst_pci_res->regions.bar_nums;
    region_num = pst_pci_res->regions.region_nums;

    bar_base = pst_pci_res->regions.pst_bars;
    region_base = pst_pci_res->regions.pst_regions;

    /* 清空regions的特定字段 */
    for (index = 0; index < region_num; index++, region_base++) {
        region_base->vaddr = NULL; /* remap 后的虚拟地址 */
        region_base->paddr = 0x0;  /* Host CPU看到的物理地址 */
        region_base->bus_addr = 0x0;
        region_base->res = NULL;
        region_base->bar_info = NULL;
        region_base->size = region_base->pci_end - region_base->pci_start + 1;
    }

    region_idx = 0;
    bar_used_size = 0;
    bar_base = pst_pci_res->regions.pst_bars;
    region_base = pst_pci_res->regions.pst_regions;

    for (index = 0; index < bar_num; index++, bar_base++) {
        for (; region_idx < region_num; region_idx++, region_base++) {
            /* BAR可用的起始地址 */
            if (bar_base->start + bar_used_size + region_base->size - 1 > bar_base->end) {
                /* 这个BAR地址空间不足 */
                pci_print_log(PCI_LOG_ERR,
                              "index:%d,region_idx:%d, start:0x%llx ,end:0x%llx, used_size:0x%x, region_size:%u\n",
                              index, region_idx, bar_base->start, bar_base->end, bar_used_size, region_base->size);
                break;
            }

            region_base->paddr = bar_base->start + bar_used_size;
            region_base->bus_addr = bar_base->bus_start + bar_used_size;
            bar_used_size += region_base->size;
            region_base->bar_info = bar_base;
            pci_print_log(PCI_LOG_INFO, "bar idx:%d, region idx:%d, region paddr:0x%llx, region_size:%u\n",
                          index, region_idx, region_base->paddr, region_base->size);
        }
    }

    if (region_idx < region_num) {
        /* 地址不够用 */
        pci_print_log(PCI_LOG_ERR, "bar address range is too small, region_idx %d < region_num %d\n",
                      region_idx, region_num);
        return -OAL_ENOMEM;
    }

    pci_print_log(PCI_LOG_INFO, "Total region num:%d, size:%d\n", region_num, bar_used_size);

    region_base = pst_pci_res->regions.pst_regions;
    for (index = 0; index < region_num; index++, region_base++) {
        if (!region_base->flag) {
            continue;
        }

        pst_res = oal_request_mem_region(region_base->paddr, region_base->size, region_base->name);
        if (pst_res == NULL) {
            goto failed_remap;
        }

        /* remap */
        if (region_base->flag & OAL_IORESOURCE_REG) {
            /* 寄存器映射成非cache段, 不需要刷cache */
            vaddr = oal_ioremap_nocache(region_base->paddr, region_base->size);
        } else {
            /* cache 段，注意要刷cache */
            vaddr = oal_ioremap(region_base->paddr, region_base->size);
        }

        if (vaddr == NULL) {
            oal_release_mem_region(region_base->paddr, region_base->size);
            goto failed_remap;
        }

        /* remap and request succ. */
        region_base->res = pst_res;
        region_base->vaddr = vaddr; /* Host Cpu 可以访问的虚拟地址 */
    }

    oal_pcie_enable_regions(pst_pci_res);

    pci_print_log(PCI_LOG_INFO, "oal_pcie_regions_init succ\n");
    return OAL_SUCC;
failed_remap:
    pci_print_log(PCI_LOG_ERR, "request mem region failed, addr:0x%llx, size:%u, name:%s\n",
                  region_base->paddr, region_base->size, region_base->name);
    oal_pcie_regions_exit(pst_pci_res);
    return -OAL_EIO;
}

OAL_STATIC void oal_pcie_bar_exit(oal_pcie_res *pst_pci_res)
{
    oal_pcie_iatu_exit(pst_pci_res);
    oal_pcie_regions_exit(pst_pci_res);
    oal_pcie_iatu_bar_exit(pst_pci_res);
}

int32_t oal_pcie_get_ca_by_pa(oal_pcie_res *pst_pci_res, uintptr_t paddr, uint64_t *cpuaddr)
{
    int32_t index;
    uint32_t region_num;
    uint64_t offset;
    oal_pcie_region *region_base;
    uintptr_t end;

    region_num = pst_pci_res->regions.region_nums;
    region_base = pst_pci_res->regions.pst_regions;

    if (cpuaddr == NULL) {
        return -OAL_EINVAL;
    }

    *cpuaddr = 0;

    if (oal_warn_on(!pst_pci_res->regions.inited)) {
        return -OAL_ENODEV;
    }

    for (index = 0; index < region_num; index++, region_base++) {
        if (region_base->vaddr == NULL) {
            continue;
        }

        end = (uintptr_t)region_base->paddr + region_base->size - 1;

        if ((paddr >= (uintptr_t)region_base->paddr) && (paddr <= end)) {
            /* 地址在范围内 */
            offset = paddr - (uintptr_t)region_base->paddr;
            *cpuaddr = region_base->cpu_start + offset;
            return OAL_SUCC;
        } else {
            continue;
        }
    }

    return -OAL_ENOMEM;
}

/*
 * 将Device Cpu看到的地址转换为 Host侧的虚拟地址,
 * 虚拟地址返回NULL为无效地址，Device Cpu地址有可能为0,
 * local ip inbound cpu address to host virtual address,
 * 函数返回非0为失败
 */
int32_t oal_pcie_inbound_ca_to_va(oal_pcie_res *pst_pci_res, uint64_t dev_cpuaddr,
                                  pci_addr_map *addr_map)
{
    int32_t index;
    uint32_t region_num;
    uint64_t offset;
    oal_pcie_region *region_base;

    region_num = pst_pci_res->regions.region_nums;
    region_base = pst_pci_res->regions.pst_regions;

    if (addr_map != NULL) {
        addr_map->pa = 0;
        addr_map->va = 0;
    }

    if (oal_unlikely(pst_pci_res->link_state < PCI_WLAN_LINK_MEM_UP)) {
        pci_print_log(PCI_LOG_WARN, "addr request 0x%llx failed, link_state:%s",
                      dev_cpuaddr, oal_pcie_get_link_state_str(pst_pci_res->link_state));
        return -OAL_EBUSY;
    }

    if (oal_warn_on(!pst_pci_res->regions.inited)) {
        return -OAL_ENODEV;
    }

    for (index = 0; index < region_num; index++, region_base++) {
        if (region_base->vaddr == NULL) {
            continue;
        }

        if ((dev_cpuaddr >= region_base->cpu_start) && (dev_cpuaddr <= region_base->cpu_end)) {
            /* 地址在范围内 */
            offset = dev_cpuaddr - region_base->cpu_start;
            if (addr_map != NULL) {
                /* 返回HOST虚拟地址 */
                addr_map->va = (uintptr_t)(region_base->vaddr + offset);
                /* 返回HOST物理地址 */
                addr_map->pa = (uintptr_t)(region_base->paddr + offset);
            }
            return OAL_SUCC;
        } else {
            continue;
        }
    }

    return -OAL_ENOMEM;
}

/* 地址转换均为线性映射，注意起始地址和大小 */
int32_t oal_pcie_inbound_va_to_ca(oal_pcie_res *pst_pci_res, uint64_t host_va, uint64_t *dev_cpuaddr)
{
    int32_t index;
    uint32_t region_num;
    uint64_t offset;
    oal_pcie_region *region_base;
    uint64_t end;
    uint64_t vaddr = host_va;

    region_num = pst_pci_res->regions.region_nums;
    region_base = pst_pci_res->regions.pst_regions;

    if (oal_warn_on(dev_cpuaddr == NULL)) {
        return -OAL_EINVAL;
    }

    *dev_cpuaddr = 0;

    if (oal_warn_on(!pst_pci_res->regions.inited)) {
        return -OAL_ENODEV;
    }

    for (index = 0; index < region_num; index++, region_base++) {
        if (region_base->vaddr == NULL) {
            continue;
        }

        end = (uintptr_t)region_base->vaddr + region_base->size - 1;

        if ((vaddr >= (uintptr_t)region_base->vaddr) && (vaddr <= end)) {
            /* 地址在范围内 */
            offset = vaddr - (uintptr_t)region_base->vaddr;
            *dev_cpuaddr = region_base->cpu_start + offset;
            return OAL_SUCC;
        } else {
            continue;
        }
    }

    return -OAL_ENOMEM;
}

/* 检查通过PCIE操作的HOST侧虚拟地址是否合法 ，是否映射过 */
int32_t oal_pcie_vaddr_isvalid(oal_pcie_res *pst_pci_res, const void *vaddr)
{
    int32_t index;
    uint32_t region_num;
    oal_pcie_region *region_base = NULL;
    if (oal_warn_on(!pst_pci_res->regions.inited)) {
        return OAL_FALSE;
    }

    region_num = pst_pci_res->regions.region_nums;
    region_base = pst_pci_res->regions.pst_regions;

    for (index = 0; index < region_num; index++, region_base++) {
        if (region_base->vaddr == NULL) {
            continue;
        }

        if (((uintptr_t)vaddr >= (uintptr_t)region_base->vaddr)
            && ((uintptr_t)vaddr < (uintptr_t)region_base->vaddr + region_base->size)) {
            return OAL_TRUE;
        }
    }

    return OAL_FALSE;
}

OAL_STATIC int32_t oal_pcie_mem_bar_init(oal_pci_dev_stru *pst_pci_dev, oal_pcie_bar_info *bar_curr)
{
    /* Get Bar Address */
    bar_curr->size = oal_pci_resource_len(pst_pci_dev, PCIE_IATU_BAR_INDEX);
    if (bar_curr->size == 0) {
        pci_print_log(PCI_LOG_ERR, "bar 1 size is zero, start:0x%lx, end:0x%lx",
                      oal_pci_resource_start(pst_pci_dev, PCIE_IATU_BAR_INDEX),
                      oal_pci_resource_end(pst_pci_dev, PCIE_IATU_BAR_INDEX));
        return -OAL_EIO;
    }

    bar_curr->end = oal_pci_resource_end(pst_pci_dev, PCIE_IATU_BAR_INDEX);
    bar_curr->start = oal_pci_resource_start(pst_pci_dev, PCIE_IATU_BAR_INDEX);
    bar_curr->bus_start = oal_pci_bus_address(pst_pci_dev, PCIE_IATU_BAR_INDEX);
    bar_curr->bar_idx = PCIE_IATU_BAR_INDEX;

    pci_print_log(PCI_LOG_INFO,
                  "preapre for bar idx:%u, phy start:0x%llx, end:0x%llx, bus address 0x%lx size:0x%x, flags:0x%lx\n",
                  PCIE_IATU_BAR_INDEX,
                  bar_curr->start,
                  bar_curr->end,
                  (uintptr_t)bar_curr->bus_start,
                  bar_curr->size,
                  oal_pci_resource_flags(pst_pci_dev, PCIE_IATU_BAR_INDEX));

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_bar_init(oal_pcie_res *pst_pci_res)
{
    int32_t index;
    int32_t ret;
    uint32_t bar_num, region_num;
    oal_pcie_bar_info *bar_base = NULL;
    oal_pcie_region *region_base = NULL;

    oal_pci_dev_stru *pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    /* 暂时只考虑1103 */
    bar_num = oal_array_size(g_en_bar_tab);
    bar_base = &g_en_bar_tab[0];

    ret = oal_pcie_get_bar_region_info(pst_pci_res, &region_base, &region_num);
    if (ret != OAL_SUCC) {
        return ret;
    }

    pci_print_log(PCI_LOG_INFO, "bar_num:%u, region_num:%u\n", bar_num, region_num);

    pst_pci_res->regions.pst_bars = bar_base;
    pst_pci_res->regions.bar_nums = bar_num;

    pst_pci_res->regions.pst_regions = region_base;
    pst_pci_res->regions.region_nums = region_num;

    /* 这里不映射，iatu配置要和映射分段对应 */
    for (index = 0; index < bar_num; index++) {
        /*
         * 获取Host分配的硬件地址资源,1103为8M大小,
         * 1103 4.7a 对应一个BAR, 5.0a 对应2个bar,
         * 其中第二个bar用于配置iatu表
         */
        oal_pcie_bar_info *bar_curr = bar_base + index;
        uint8_t bar_idx = bar_curr->bar_idx;

        /* pci resource built in pci_read_bases kernel. */
        bar_curr->start = oal_pci_resource_start(pst_pci_dev, bar_idx);
        bar_curr->end = oal_pci_resource_end(pst_pci_dev, bar_idx);
        bar_curr->bus_start = oal_pci_bus_address(pst_pci_dev, bar_idx);
        bar_curr->size = oal_pci_resource_len(pst_pci_dev, bar_idx);

        pci_print_log(PCI_LOG_INFO,
                      "preapre for bar idx:%u, \
                      phy start:0x%llx, end:0x%llx, bus address 0x%lx size:0x%x, flags:0x%lx\n",
                      bar_idx,
                      bar_curr->start,
                      bar_curr->end,
                      (uintptr_t)bar_curr->bus_start,
                      bar_curr->size,
                      oal_pci_resource_flags(pst_pci_dev, bar_idx));
    }

    /* 是否支持BAR1 */
    if (pst_pci_res->chip_info.membar_support == OAL_TRUE) {
        /* Get Bar Address */
        oal_pcie_bar_info *bar_curr = &pst_pci_res->st_iatu_bar.st_bar_info;
        ret = oal_pcie_mem_bar_init(pst_pci_dev, bar_curr);
        if (ret != OAL_SUCC) {
            return ret;
        }
    }

    ret = oal_pcie_iatu_bar_init(pst_pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = oal_pcie_regions_init(pst_pci_res);
    if (ret != OAL_SUCC) {
        oal_pcie_iatu_bar_exit(pst_pci_res);
        return ret;
    }

    ret = oal_pcie_iatu_init(pst_pci_res);
    if (ret != OAL_SUCC) {
        oal_pcie_regions_exit(pst_pci_res);
        oal_pcie_iatu_bar_exit(pst_pci_res);
        return ret;
    }
    pci_print_log(PCI_LOG_INFO, "bar init succ");
    return OAL_SUCC;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
int32_t oal_pcie_dual_bar_init(oal_pcie_res *pst_pci_res, oal_pci_dev_stru *pst_pci_dev)
{
    return oal_pcie_mem_bar_init(pst_pci_dev, &pst_pci_res->st_iatu_dual_bar.st_bar_info);
}
#endif

int32_t oal_pcie_device_check_alive(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    uint32_t value;
    pci_addr_map addr_map;

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res,
                                    pst_pci_res->chip_info.addr_info.glb_ctrl, &addr_map);
    if (oal_unlikely(ret != OAL_SUCC)) {
        /* share mem 地址未映射! */
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "can not found mem map for dev cpu address 0x%x\n", 0x50000000);
        return -OAL_EFAIL;
    }

    value = oal_readl((void *)addr_map.va);
    if (value == 0x101) {
        return OAL_SUCC;
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "pcie maybe linkdown, glbctrl=0x%x", value);
        return -OAL_EFAIL;
    }
}

void oal_pcie_print_ringbuf_info(pcie_ringbuf *pst_ringbuf, pci_log_type level)
{
    if (oal_warn_on(pst_ringbuf == NULL)) {
        return;
    }

    /* dump the ringbuf info */
    pci_print_log(level, "ringbuf[0x%p] idx:%u, rd:%u, wr:%u, size:%u, item_len:%u, item_mask:0x%x, base_addr:0x%llx",
                  pst_ringbuf,
                  pst_ringbuf->idx,
                  pst_ringbuf->rd,
                  pst_ringbuf->wr,
                  pst_ringbuf->size,
                  pst_ringbuf->item_len,
                  pst_ringbuf->item_mask,
                  pst_ringbuf->base_addr);
}

/* first power init */
int32_t oal_pcie_transfer_res_init(oal_pcie_res *pst_pci_res)
{
    int32_t ret = -OAL_ENODEV;

    if (pst_pci_res->chip_info.edma_support == OAL_TRUE) {
        ret = oal_pcie_transfer_edma_res_init(pst_pci_res);
    } else if (pst_pci_res->chip_info.ete_support == OAL_TRUE) {
        ret = oal_ete_transfer_res_init(pst_pci_res);
    }

    return ret;
}

void oal_pcie_transfer_res_exit(oal_pcie_res *pst_pci_res)
{
    if (pst_pci_res->chip_info.edma_support == OAL_TRUE) {
        oal_pcie_transfer_edma_res_exit(pst_pci_res);
    } else if (pst_pci_res->chip_info.ete_support == OAL_TRUE) {
        oal_ete_transfer_res_exit(pst_pci_res);
    }
}

/* 配置BAR,IATU等设备资源 */
int32_t oal_pcie_dev_init(oal_pcie_res *pci_res)
{
    int32_t ret;
    oal_pci_dev_stru *pci_dev = pcie_res_to_dev(pci_res);

    if (oal_warn_on(pci_dev == NULL)) {
        return -OAL_ENODEV;
    }

    ret = oal_pcie_bar_init(pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = oal_pcie_ctrl_base_address_init(pci_res);
    if (ret != OAL_SUCC) {
        oal_pcie_bar_exit(pci_res);
        return ret;
    }

    return OAL_SUCC;
}
void oal_pcie_dev_deinit(oal_pcie_res *pci_res)
{
    oal_pci_dev_stru *pci_dev = pcie_res_to_dev(pci_res);
    if (oal_warn_on(pci_dev == NULL)) {
        return;
    }
    oal_pcie_ctrl_base_address_exit(pci_res);
    oal_pcie_bar_exit(pci_res);
}

void oal_pcie_release_rx_netbuf(oal_pcie_res *pci_res, oal_netbuf_stru *netbuf)
{
    pcie_cb_dma_res *cb_res = NULL;
    oal_pci_dev_stru *pci_dev = NULL;
    if (oal_warn_on(netbuf == NULL)) {
        return;
    }

    if (oal_warn_on(pci_res == NULL)) {
        declare_dft_trace_key_info("pcie release rx netbuf", OAL_DFT_TRACE_FAIL);
        oal_netbuf_free(netbuf);
        return;
    }

    pci_dev = pcie_res_to_dev(pci_res);
    if (oal_warn_on(pci_dev == NULL)) {
        declare_dft_trace_key_info("pcie release rx netbuf", OAL_DFT_TRACE_FAIL);
        oal_netbuf_free(netbuf);
        return;
    }

    cb_res = (pcie_cb_dma_res *)oal_netbuf_cb(netbuf);
    if (oal_likely((cb_res->paddr.addr != 0) && (cb_res->len != 0))) {
        dma_unmap_single(&pci_dev->dev, (dma_addr_t)cb_res->paddr.addr, cb_res->len, PCI_DMA_FROMDEVICE);
    } else {
        declare_dft_trace_key_info("pcie release rx netbuf", OAL_DFT_TRACE_FAIL);
    }

    oal_netbuf_free(netbuf);
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static inline void oal_pcie_invalid_rx_buf_dcache(oal_pci_dev_stru *pci_dev, dma_addr_t dma_addr, int32_t len)
{
    oal_pci_cache_inv(pci_dev, dma_addr, len);
}
#endif

static int32_t oal_pcie_rx_databuf_verify(oal_pci_dev_stru *pci_dev, oal_netbuf_stru *netbuf)
{
    if (g_pcie_dma_data_rx_check != 0) {
        // 规避方案打开
        return OAL_SUCC;
    }

    if ((*(volatile uint32_t*)oal_netbuf_data(netbuf)) != PCIE_RX_TRANS_FLAG) {
        // 默认 认为传输完成，after unmap
        return OAL_SUCC; // succ, buf had rewrite by pcie
    }

    oam_error_log0(0, OAM_SF_ANY, "pcie rx netbuf check failed, invalid netbuf, loss pkt!");
    declare_dft_trace_key_info("invalid_pcie_rx_netbuf", OAL_DFT_TRACE_FAIL);
    oal_print_hex_dump((uint8_t *)(oal_netbuf_data(netbuf)),
                       HCC_HDR_TOTAL_LEN, HEX_DUMP_GROUP_SIZE, "err_hdr ");

    return -OAL_EFAIL;
}

void oal_pcie_load_rx_buf_cache(oal_netbuf_stru *netbuf)
{
    volatile uint32_t j;
    uint32_t len = oal_netbuf_len(netbuf);
    volatile uint8_t *data = (volatile uint8_t*)oal_netbuf_data(netbuf);
    for (j = 0; j < len; j++) {
        *(data + j);
    }
}

#define PCIE_RX_BUF_PENDING_WAIT_CNT 1000
static int32_t oal_pcie_rx_databuf_verify_fix(oal_pci_dev_stru *pci_dev, oal_netbuf_stru *netbuf,
                                              dma_addr_t dma_addr, int32_t len)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    uint32_t i;
    unsigned long flags;

    if (g_pcie_dma_data_rx_check == 0) {
        // 规避方案关闭
        return OAL_SUCC;
    }

    local_irq_save(flags);
    // 这里防止CPU投机访问已经Load Cache需要invalid一次
    oal_pcie_invalid_rx_buf_dcache(pci_dev, dma_addr, len);
    /* pcie rx netbuf自己申请，首地址保证对齐 */
    if ((*(volatile uint32_t*)oal_netbuf_data(netbuf)) != PCIE_RX_TRANS_FLAG) {
        // 默认 认为传输完成，后面还有一次invalid cache, unmap操作
        local_irq_restore(flags);
        return OAL_SUCC; // succ, buf had rewrite by pcie
    }

    oal_pcie_invalid_rx_buf_dcache(pci_dev, dma_addr, len);
    local_irq_restore(flags);

    declare_dft_trace_key_info("pcie_rx_buf_pending_in_ddr", OAL_DFT_TRACE_OTHER);

    /* 等到ddr数据刷新,规避ddr 反压导致的pending问题 */
    local_irq_save(flags);
    for (i = 0; i < PCIE_RX_BUF_PENDING_WAIT_CNT; i++) {
        oal_pcie_invalid_rx_buf_dcache(pci_dev, dma_addr, len);
        local_irq_restore(flags);
        /* 防止关中断过久，保证cache缓存时中断是关闭状态 */
        local_irq_save(flags);
        if ((*(volatile uint32_t*)oal_netbuf_data(netbuf)) != PCIE_RX_TRANS_FLAG) {
            break;
        }
    }

    if (i == PCIE_RX_BUF_PENDING_WAIT_CNT) {
        // timeout, ddr invalid, rx failed
        declare_dft_trace_key_info("invalid_pcie_rx_netbuf_fix_failed", OAL_DFT_TRACE_FAIL);
        oal_print_hex_dump((uint8_t *)(oal_netbuf_data(netbuf)),
                           HCC_HDR_TOTAL_LEN, HEX_DUMP_GROUP_SIZE, "err_hdr ");
        local_irq_restore(flags);
        oam_error_log0(0, OAM_SF_ANY, "pcie rx netbuf check failed, try to fix failed!");
        return -OAL_ETIMEDOUT;
    }

    // 此处说明PCIE已经把buf 头4字节写完, 无效cache, 重新装载整个报文到cache
    oal_pcie_invalid_rx_buf_dcache(pci_dev, dma_addr, len);

    /* cpu 主动load一次cache,保证pcie把所有报文内容写入ddr
    * pcie burst写一般比cpu single读要快很多 */
    oal_pcie_load_rx_buf_cache(netbuf);

    oal_pcie_invalid_rx_buf_dcache(pci_dev, dma_addr, len);
    local_irq_restore(flags);

    /* wait pcie pending data to ddr succ, invalid cache, try to read data from ddr */
    declare_dft_trace_key_info("pcie_rx_buf_pending_retry_succ", OAL_DFT_TRACE_OTHER);
#endif

    return OAL_SUCC;
}

/* 向Hcc层提交收到的netbuf */
void oal_pcie_rx_netbuf_submit(oal_pcie_res *pci_res, oal_netbuf_stru *netbuf)
{
    int32_t ret1, ret2;
    struct hcc_handler *hcc = NULL;
    oal_pci_dev_stru *pci_dev = NULL;
    pcie_cb_dma_res *cb_res = NULL;
    oal_pcie_linux_res *pci_lres = NULL;

    pci_dev = pcie_res_to_dev(pci_res);
    if (oal_unlikely(pci_dev == NULL)) {
        goto release_netbuf;
    }

    pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pci_dev);
    if (oal_unlikely(pci_lres == NULL)) {
        pci_print_log(PCI_LOG_ERR, "lres is null");
        goto release_netbuf;
    }

    if (oal_unlikely(pci_lres->pst_bus == NULL)) {
        pci_print_log(PCI_LOG_ERR, "lres's bus is null");
        goto release_netbuf;
    }

    if (oal_unlikely(hbus_to_dev(pci_lres->pst_bus) == NULL)) {
        pci_print_log(PCI_LOG_ERR, "lres's dev is null");
        goto release_netbuf;
    }

    hcc = hbus_to_hcc(pci_lres->pst_bus);
    if (oal_unlikely(hcc == NULL)) {
        pci_print_log(PCI_LOG_ERR, "lres's hcc is null");
        goto release_netbuf;
    }

    cb_res = (pcie_cb_dma_res *)oal_netbuf_cb(netbuf);
    if (oal_unlikely((cb_res->paddr.addr == 0) || (cb_res->len == 0))) {
        goto release_netbuf;
    }

    /* verify rx done buf's hdr, transfer from pcie dma */
    ret1 = oal_pcie_rx_databuf_verify_fix(pci_dev, netbuf, (dma_addr_t)cb_res->paddr.addr,
                                          (int32_t)cb_res->len);

    /* unmap pcie dma addr */
    dma_unmap_single(&pci_dev->dev, (dma_addr_t)cb_res->paddr.addr, cb_res->len, PCI_DMA_FROMDEVICE);

    /* DDR pending 规避方案关闭 */
    ret2 = oal_pcie_rx_databuf_verify(pci_dev, netbuf);
    if (oal_likely((ret1 == OAL_SUCC) && (ret2 == OAL_SUCC))) {
        hcc_rx_submit(hcc, netbuf);
    } else {
        oal_netbuf_free(netbuf);
        declare_dft_trace_key_info("pcie_rx_netbuf_loss", OAL_DFT_TRACE_FAIL);
    }

    return;

release_netbuf:
    oal_pcie_release_rx_netbuf(pci_res, netbuf);
    declare_dft_trace_key_info("pcie release rx netbuf", OAL_DFT_TRACE_OTHER);
    return;
}

OAL_STATIC int32_t oal_pcie_unmap_tx_netbuf(oal_pcie_res *pst_pci_res, oal_netbuf_stru *pst_netbuf)
{
    /* dma_addr 存放在CB字段里 */
    pcie_cb_dma_res st_cb_dma;
    oal_pci_dev_stru *pst_pci_dev;
    int32_t ret;

    pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    /* 不是从CB的首地址开始，必须拷贝，对齐问题。 */
    ret = memcpy_s(&st_cb_dma, sizeof(pcie_cb_dma_res),
                   (uint8_t *)oal_netbuf_cb(pst_netbuf) + sizeof(struct hcc_tx_cb_stru),
                   sizeof(pcie_cb_dma_res));
    if (ret != EOK) {
        pci_print_log(PCI_LOG_ERR, "get dma addr from cb filed failed");
        return -OAL_EFAIL;
    }
#ifdef _PRE_PLAT_FEATURE_PCIE_DEBUG
    /* Debug */
    memset_s((uint8_t *)oal_netbuf_cb(pst_netbuf) + sizeof(struct hcc_tx_cb_stru),
             oal_netbuf_cb_size() - sizeof(struct hcc_tx_cb_stru), 0, sizeof(st_cb_dma));
#endif

    /* unmap pcie dma addr */
    if (oal_likely((st_cb_dma.paddr.addr != 0) && (st_cb_dma.len != 0))) {
        dma_unmap_single(&pst_pci_dev->dev, (dma_addr_t)st_cb_dma.paddr.addr, st_cb_dma.len, PCI_DMA_TODEVICE);
    } else {
        declare_dft_trace_key_info("pcie tx netbuf free fail", OAL_DFT_TRACE_FAIL);
        oal_print_hex_dump((uint8_t *)oal_netbuf_cb(pst_netbuf), oal_netbuf_cb_size(),
                           HEX_DUMP_GROUP_SIZE, "invalid cb: ");
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

void oal_pcie_tx_netbuf_free(oal_pcie_res *pci_res, oal_netbuf_stru *netbuf)
{
    oal_pcie_unmap_tx_netbuf(pci_res, netbuf);

    hcc_tx_netbuf_free(netbuf);
}

/* ringbuf functions */
uint32_t oal_pcie_ringbuf_freecount(pcie_ringbuf *pst_ringbuf)
{
    /* 无符号，已经考虑了翻转 */
    uint32_t len = pst_ringbuf->size - (pst_ringbuf->wr - pst_ringbuf->rd);
    if (len == 0) {
        return 0;
    }

    if (len % pst_ringbuf->item_len) {
        pci_print_log(PCI_LOG_ERR, "oal_pcie_ringbuf_freecount, size:%u, wr:%u, rd:%u",
                      pst_ringbuf->size,
                      pst_ringbuf->wr,
                      pst_ringbuf->rd);
        return 0;
    }

    if (pst_ringbuf->item_mask) {
        /* item len 如果是2的N次幂，则移位 */
        len = len >> pst_ringbuf->item_mask;
    } else {
        len /= pst_ringbuf->item_len;
    }
    return len;
}

int32_t oal_pcie_check_link_state(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    pci_addr_map addr_map;
    pcie_dev_ptr share_mem_address; /* Device cpu地址 */
    oal_pci_dev_stru *pst_pci_dev;

    pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, pst_pci_res->chip_info.addr_info.sharemem_addr, &addr_map);
    if (oal_unlikely(ret != OAL_SUCC)) {
        /* share mem 地址未映射! */
        pci_print_log(PCI_LOG_ERR, "can not found mem map for dev cpu address 0x%x\n",
                      pst_pci_res->chip_info.addr_info.sharemem_addr);
        return OAL_FALSE;
    }

    /* Get sharemem's dev_cpu address */
    oal_pcie_io_trans((uintptr_t)&share_mem_address, addr_map.va, sizeof(share_mem_address));

    if (share_mem_address == 0xFFFFFFFF) {
        uint32_t version = 0;

        declare_dft_trace_key_info("pcie_detect_linkdown", OAL_DFT_TRACE_EXCEP);

        ret = oal_pci_read_config_dword(pcie_res_to_dev(pst_pci_res), 0x0, &version);

        /* if pci config succ, pcie link still up, but pci master is down,
         * rc mem port is issue or ep axi bus is gating
         * if pci config failed, pcie maybe link down */
        pci_print_log(PCI_LOG_INFO, "read pci version 0x%8x ret=%d, host wakeup dev gpio:%d", version, ret,
                      hcc_bus_get_sleep_state(((oal_pcie_linux_res *)oal_pci_get_drvdata(
                      pcie_res_to_dev(pst_pci_res)))->pst_bus));
        do {
            oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pcie_res_to_dev(pst_pci_res));

            oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_DOWN);

#if defined(_PRE_CONFIG_GPIO_TO_SSI_DEBUG)
            if (board_get_wlan_wkup_gpio_val() == 0) {
                (void)ssi_dump_err_regs(SSI_ERR_PCIE_CHECK_LINK_FAIL);
            } else {
                pci_print_log(PCI_LOG_INFO, "dev wakeup gpio is high, dev maybe panic");
            }
#endif
            oal_pci_disable_device(pcie_res_to_dev(pst_pci_res));
            oal_disable_pcie_irq(pst_pci_lres);
        } while (0);
        return OAL_FALSE;
    } else {
        return OAL_TRUE;
    }
}

void oal_pcie_share_mem_res_unmap(oal_pcie_res *pst_pci_res)
{
    memset_s((void *)&pst_pci_res->dev_share_mem, sizeof(pst_pci_res->dev_share_mem),
             0, sizeof(pst_pci_res->dev_share_mem));
}

/* 调用必须在iATU配置, pcie device 使能之后， */
int32_t oal_pcie_share_mem_res_map(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    void *pst_share_mem_vaddr = NULL;
    pcie_dev_ptr share_mem_address = 0xFFFFFFFF; /* Device cpu地址 */
    pci_addr_map addr_map, share_mem_map;
    unsigned long timeout, timeout1;
    oal_pci_dev_stru *pst_pci_dev = NULL;

    pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    /* 忙等50ms 若超时 再等10S 超时 */
    timeout = jiffies + msecs_to_jiffies(hi110x_get_emu_timeout(50));     /* 50ms each timeout */
    timeout1 = jiffies + msecs_to_jiffies(hi110x_get_emu_timeout(10000)); /* 10000ms total timeout */

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, pst_pci_res->chip_info.addr_info.sharemem_addr, &addr_map);
    if (ret != OAL_SUCC) {
        /* share mem 地址未映射! */
        pci_print_log(PCI_LOG_ERR, "can not found mem map for dev cpu address 0x%x\n",
                      pst_pci_res->chip_info.addr_info.sharemem_addr);
        return ret;
    }

    pst_share_mem_vaddr = (void *)addr_map.va;

    pci_print_log(PCI_LOG_DBG, "device address:0x%x = va:0x%lx",
                  pst_pci_res->chip_info.addr_info.sharemem_addr, addr_map.va);

    forever_loop() {
        /* Get sharemem's dev_cpu address */
        oal_pcie_io_trans((uintptr_t)&share_mem_address, (uintptr_t)pst_share_mem_vaddr, sizeof(share_mem_address));

        /* 通过检查地址转换可以判断读出的sharemem地址是否是有效值 */
        ret = oal_pcie_inbound_ca_to_va(pst_pci_res, share_mem_address, &share_mem_map);
        if (ret == OAL_SUCC) {
            /* Device 初始化完成  & PCIE 通信正常 */
            if (share_mem_address != 0) {
                if (0xFFFFFFFF != share_mem_address) {
                    pci_print_log(PCI_LOG_INFO, "share_mem_address 0x%x", share_mem_address);
                    ret = OAL_SUCC;
                    break;
                } else {
                    pci_print_log(PCI_LOG_ERR, "invalid sharemem address 0x%x, maybe linkdown", share_mem_address);
                    return -OAL_ENODEV;
                }
            }
        }

        if (!time_after(jiffies, timeout)) {
            cpu_relax();
            continue; /* 未超时，继续 */
        }

        /* 50ms 超时, 开始10S超时探测 */
        if (!time_after(jiffies, timeout1)) {
            oal_msleep(1);
            continue; /* 未超时，继续 */
        } else {
            /* 10s+50ms 超时，退出 */
            pci_print_log(PCI_LOG_ERR, "share_mem_address 0x%x, jiffies:0x%lx, timeout:0x%lx, timeout1:0x%lx",
                          share_mem_address, jiffies, timeout, timeout1);
            ret = -OAL_ETIMEDOUT;
            break;
        }
    }

    if (share_mem_map.va != 0 && (ret == OAL_SUCC)) {
        pst_pci_res->dev_share_mem.va = share_mem_map.va;
        pst_pci_res->dev_share_mem.pa = share_mem_map.pa;
        pci_print_log(PCI_LOG_DBG, "share mem va:0x%lx, pa:0x%lx",
                      pst_pci_res->dev_share_mem.va, pst_pci_res->dev_share_mem.pa);
        return OAL_SUCC;
    }

    /* 此处失败可能的原因(若host第一次加载就失败可能是原因1, 否者可能是原因2):
     * 1.下载完后boot跳转到firmware main函数失败(firmware镜像问题:ram代码跑飞比如切高频失败等)
     * 2.下载之前的EN gpio复位操作失效，重复下载了firmware */
    pci_print_log(PCI_LOG_INFO,
                  "pcie boot timeout, jump firmware failed or subsys reset failed, 0x%x",
                  share_mem_address);

    (void)ssi_dump_err_regs(SSI_ERR_PCIE_WAIT_BOOT_TIMEOUT);

    return -OAL_EFAIL;
}

/* device shared mem write */
int32_t oal_pcie_write_dsm32(oal_pcie_res *pst_pci_res, pcie_shared_device_addr_type type, uint32_t val)
{
    if (oal_warn_on((uint32_t)type >= (uint32_t)PCIE_SHARED_ADDR_BUTT)) {
        pci_print_log(PCI_LOG_WARN, "invalid device addr type:%d", type);
        return -OAL_EINVAL;
    }

    if (oal_warn_on(pst_pci_res == NULL)) {
        pci_print_log(PCI_LOG_ERR, "pci res is null");
        return -OAL_ENODEV;
    }

    if (oal_warn_on(pst_pci_res->st_device_shared_addr_map[type].va == 0)) {
        pci_print_log(PCI_LOG_ERR, "dsm type:%d va is null", type);
        return -OAL_ENODEV;
    }

    oal_writel(val, (void *)pst_pci_res->st_device_shared_addr_map[type].va);

    return OAL_SUCC;
}

/* device shared mem read */
int32_t oal_pcie_read_dsm32(oal_pcie_res *pst_pci_res, pcie_shared_device_addr_type type, uint32_t *val)
{
    if (oal_warn_on((uint32_t)type >= (uint32_t)PCIE_SHARED_ADDR_BUTT)) {
        pci_print_log(PCI_LOG_WARN, "invalid device addr type:%d", type);
        return -OAL_EINVAL;
    }

    if (oal_warn_on(pst_pci_res == NULL)) {
        pci_print_log(PCI_LOG_ERR, "pci res is null");
        return -OAL_ENODEV;
    }

    if (oal_warn_on(pst_pci_res->st_device_shared_addr_map[type].va == 0)) {
        pci_print_log(PCI_LOG_ERR, "dsm type:%d va is null", type);
        return -OAL_ENODEV;
    }

    *val = oal_readl((void *)pst_pci_res->st_device_shared_addr_map[type].va);

    return OAL_SUCC;
}

int32_t oal_pcie_set_device_soft_fifo_enable(oal_pcie_res *pst_pci_res)
{
    if (pst_pci_res->revision >= PCIE_REVISION_5_00A) {
        return oal_pcie_write_dsm32(pst_pci_res, PCIE_SHARED_SOFT_FIFO_ENABLE, !!g_pcie_soft_fifo_enable);
    } else {
        return OAL_SUCC;
    }
}

int32_t oal_pcie_set_device_ringbuf_bugfix_enable(oal_pcie_res *pst_pci_res)
{
    return oal_pcie_write_dsm32(pst_pci_res, PCIE_SHARED_RINGBUF_BUGFIX, !!g_pcie_ringbuf_bugfix_enable);
}

int32_t oal_pcie_set_device_aspm_dync_disable(oal_pcie_res *pst_pci_res, uint32_t disable)
{
    uint32_t value = 0;
    int32_t ret = oal_pcie_write_dsm32(pst_pci_res, PCIE_SHARED_ASPM_DYNC_CTL, disable);
    oal_pcie_read_dsm32(pst_pci_res, PCIE_SHARED_ASPM_DYNC_CTL, &value);
    oal_reference(value);
    return ret;
}

int32_t oal_pcie_set_device_dma_check_enable(oal_pcie_res *pst_pci_res)
{
    return oal_pcie_write_dsm32(pst_pci_res, PCIE_SHARED_SOFT_DMA_CHECK, !!g_pcie_dma_data_check_enable);
}

/* 电压拉偏初始化 */
int32_t oal_pcie_voltage_bias_init(oal_pcie_res *pst_pci_res)
{
    if (pst_pci_res->chip_info.cb.pcie_voltage_bias_init == NULL) {
        return -OAL_ENODEV;
    }
    return pst_pci_res->chip_info.cb.pcie_voltage_bias_init(pst_pci_res);
}

int32_t oal_pcie_copy_to_device_by_dword(oal_pcie_res *pst_pci_res,
                                         void *ddr_address,
                                         unsigned long start,
                                         uint32_t data_size)
{
    uint32_t i;
    int32_t ret;
    uint32_t value;
    unsigned long length;
    pci_addr_map addr_map;

    length = (unsigned long)data_size;

    if (oal_unlikely(((uintptr_t)ddr_address & 0x3) || (data_size & 0x3))) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "ddr address %lu, length 0x%lx is invalid",
                             (uintptr_t)ddr_address, length);
        return -OAL_EINVAL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start + length - 1, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx + length:0x%lx  invalid", start, length);
        return ret;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, start, &addr_map);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "cpu address start:0x%lx", start);
        return ret;
    }

    for (i = 0; i < (uint32_t)length; i += sizeof(uint32_t)) { /* 每次偏移4字节 */
        value = oal_readl(ddr_address + i);
        oal_writel(value, (void *)(addr_map.va + i));
    }

    return (int32_t)data_size;
}

OAL_STATIC int32_t oal_pcie_device_shared_addr_res_map(oal_pcie_res *pst_pci_res,
                                                       pcie_share_mem_stru *pst_share_mem)
{
    int32_t ret;
    int32_t i;

    pci_addr_map st_map;

    for (i = 0; i < PCIE_SHARED_ADDR_BUTT; i++) {
        if (pst_share_mem->device_addr[i] == 0) {
            pci_print_log(PCI_LOG_DBG, "type:%d , device addr is zero", i);
            continue;
        }

        ret = oal_pcie_inbound_ca_to_va(pst_pci_res, pst_share_mem->device_addr[i], &st_map);
        if (ret != OAL_SUCC) {
            pci_print_log(PCI_LOG_ERR, "convert device addr type:%d, addr:0x%x failed, ret=%d",
                          i, pst_share_mem->device_addr[i], ret);
            return -OAL_ENOMEM;
        }

        memcpy_s(&pst_pci_res->st_device_shared_addr_map[i], sizeof(pci_addr_map), &st_map, sizeof(pci_addr_map));
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_shared_addr_res_unmap(oal_pcie_res *pst_pci_res)
{
    memset_s(&pst_pci_res->st_device_shared_addr_map, sizeof(pst_pci_res->st_device_shared_addr_map),
             0, sizeof(pst_pci_res->st_device_shared_addr_map));
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_comm_ringbuf_res_unmap(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    for (i = 0; i < PCIE_COMM_RINGBUF_BUTT; i++) {
        memset_s(&pst_pci_res->st_ringbuf_res.comm_rb_res[i].ctrl_daddr,
                 sizeof(pst_pci_res->st_ringbuf_res.comm_rb_res[i].ctrl_daddr),
                 0, sizeof(pst_pci_res->st_ringbuf_res.comm_rb_res[i].ctrl_daddr));
        memset_s(&pst_pci_res->st_ringbuf_res.comm_rb_res[i].data_daddr,
                 sizeof(pst_pci_res->st_ringbuf_res.comm_rb_res[i].data_daddr),
                 0, sizeof(pst_pci_res->st_ringbuf_res.comm_rb_res[i].data_daddr));
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_comm_ringbuf_res_map(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    int32_t ret;
    pci_addr_map st_map; /* DEVICE CPU地址 */

    for (i = 0; i < PCIE_COMM_RINGBUF_BUTT; i++) {
        if (pst_pci_res->st_ringbuf.st_ringbuf[i].base_addr == 0) {
            /* ringbuf invalid */
            continue;
        }

        /* get ringbuf base_addr */
        ret = oal_pcie_inbound_ca_to_va(pst_pci_res, pst_pci_res->st_ringbuf.st_ringbuf[i].base_addr, &st_map);
        if (ret != OAL_SUCC) {
            pci_print_log(PCI_LOG_ERR, "invalid comm ringbuf base address 0x%llx, rb id:%d map failed, ret=%d\n",
                          pst_pci_res->st_ringbuf.st_ringbuf[i].base_addr, i, ret);
            return -OAL_ENOMEM;
        }

        pci_print_log(PCI_LOG_DBG, "comm ringbuf %d base address is 0x%llx",
                      i, pst_pci_res->st_ringbuf.st_ringbuf[i].base_addr);

        /* comm ringbuf data 所在DMA地址 */
        memcpy_s((void *)&pst_pci_res->st_ringbuf_res.comm_rb_res[i].data_daddr,
                 sizeof(pci_addr_map), (void *)&st_map, sizeof(pci_addr_map));

        /* comm ringbuf ctrl address */
        pst_pci_res->st_ringbuf_res.comm_rb_res[i].ctrl_daddr.va = pst_pci_res->st_ringbuf_map.va +
                                                                   OAL_OFFSET_OF(pcie_ringbuf_res, st_ringbuf[i]);
        pst_pci_res->st_ringbuf_res.comm_rb_res[i].ctrl_daddr.pa = pst_pci_res->st_ringbuf_map.pa +
                                                                   OAL_OFFSET_OF(pcie_ringbuf_res, st_ringbuf[i]);
    }

    return OAL_SUCC;
}

void oal_pcie_ringbuf_res_unmap(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    memset_s(&pst_pci_res->st_ringbuf_map, sizeof(pst_pci_res->st_ringbuf_map),
             0, sizeof(pst_pci_res->st_ringbuf_map));
    memset_s(&pst_pci_res->st_ringbuf, sizeof(pst_pci_res->st_ringbuf), 0, sizeof(pst_pci_res->st_ringbuf));
    memset_s(&pst_pci_res->st_rx_res.ringbuf_data_dma_addr, sizeof(pst_pci_res->st_rx_res.ringbuf_data_dma_addr),
             0, sizeof(pst_pci_res->st_rx_res.ringbuf_data_dma_addr));
    memset_s(&pst_pci_res->st_rx_res.ringbuf_ctrl_dma_addr, sizeof(pst_pci_res->st_rx_res.ringbuf_ctrl_dma_addr),
             0, sizeof(pst_pci_res->st_rx_res.ringbuf_ctrl_dma_addr));
    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        memset_s(&pst_pci_res->st_tx_res[i].ringbuf_data_dma_addr,
                 sizeof(pst_pci_res->st_tx_res[i].ringbuf_data_dma_addr),
                 0, sizeof(pst_pci_res->st_tx_res[i].ringbuf_data_dma_addr));
        memset_s(&pst_pci_res->st_tx_res[i].ringbuf_ctrl_dma_addr,
                 sizeof(pst_pci_res->st_tx_res[i].ringbuf_ctrl_dma_addr),
                 0, sizeof(pst_pci_res->st_tx_res[i].ringbuf_ctrl_dma_addr));
    }

    memset_s(&pst_pci_res->st_message_res.d2h_res.ringbuf_ctrl_dma_addr,
             sizeof(pst_pci_res->st_message_res.d2h_res.ringbuf_ctrl_dma_addr),
             0, sizeof(pst_pci_res->st_message_res.d2h_res.ringbuf_ctrl_dma_addr));
    memset_s(&pst_pci_res->st_message_res.d2h_res.ringbuf_data_dma_addr,
             sizeof(pst_pci_res->st_message_res.d2h_res.ringbuf_data_dma_addr),
             0, sizeof(pst_pci_res->st_message_res.d2h_res.ringbuf_data_dma_addr));

    memset_s(&pst_pci_res->st_message_res.h2d_res.ringbuf_ctrl_dma_addr,
             sizeof(pst_pci_res->st_message_res.h2d_res.ringbuf_ctrl_dma_addr),
             0, sizeof(pst_pci_res->st_message_res.h2d_res.ringbuf_ctrl_dma_addr));
    memset_s(&pst_pci_res->st_message_res.h2d_res.ringbuf_data_dma_addr,
             sizeof(pst_pci_res->st_message_res.h2d_res.ringbuf_data_dma_addr),
             0, sizeof(pst_pci_res->st_message_res.h2d_res.ringbuf_data_dma_addr));

    memset_s(&pst_pci_res->st_device_stat, sizeof(pst_pci_res->st_device_stat),
             0, sizeof(pst_pci_res->st_device_stat));

    oal_pcie_device_shared_addr_res_unmap(pst_pci_res);

    oal_pcie_comm_ringbuf_res_unmap(pst_pci_res);
}

int32_t oal_pcie_ringbuf_h2d_refresh(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    int32_t i;
    pcie_share_mem_stru st_share_mem;
    pci_addr_map st_map; /* DEVICE CPU地址 */

    oal_pcie_io_trans((uintptr_t)&st_share_mem, pst_pci_res->dev_share_mem.va, sizeof(pcie_share_mem_stru));

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, st_share_mem.ringbuf_res_paddr, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "invalid ringbuf device address 0x%x, map failed\n",
                      st_share_mem.ringbuf_res_paddr);
        oal_print_hex_dump((uint8_t *)&st_share_mem, sizeof(st_share_mem),
                           HEX_DUMP_GROUP_SIZE, "st_share_mem: ");
        return -OAL_ENOMEM;
    }

    /* h->h */
    memcpy_s(&pst_pci_res->st_ringbuf_map, sizeof(pci_addr_map), &st_map, sizeof(pci_addr_map));

    /* device的ringbuf管理结构同步到Host */
    /* 这里重新刷新h2d ringbuf 指针 */
    oal_pcie_io_trans((uintptr_t)&pst_pci_res->st_ringbuf, (uintptr_t)pst_pci_res->st_ringbuf_map.va,
                      sizeof(pst_pci_res->st_ringbuf));

    /* 初始化RX BUFF */
    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        unsigned long offset;
        ret = oal_pcie_inbound_ca_to_va(pst_pci_res, pst_pci_res->st_ringbuf.st_h2d_buf[i].base_addr, &st_map);
        if (ret != OAL_SUCC) {
            pci_print_log(PCI_LOG_ERR, "invalid d2h ringbuf[%d] base address 0x%llx, map failed, ret=%d\n",
                          i, pst_pci_res->st_ringbuf.st_h2d_buf[i].base_addr, ret);
            return -OAL_ENOMEM;
        }
        memcpy_s(&pst_pci_res->st_tx_res[i].ringbuf_data_dma_addr,
                 sizeof(pci_addr_map), &st_map, sizeof(pci_addr_map));
        offset = ((uintptr_t)&pst_pci_res->st_ringbuf.st_h2d_buf[i]) - ((uintptr_t)&pst_pci_res->st_ringbuf);
        pst_pci_res->st_tx_res[i].ringbuf_ctrl_dma_addr.pa = pst_pci_res->st_ringbuf_map.pa + offset;
        pst_pci_res->st_tx_res[i].ringbuf_ctrl_dma_addr.va = pst_pci_res->st_ringbuf_map.va + offset;
    }

    return OAL_SUCC;
}

/* 初始化Host ringbuf 和 Device ringbuf 的映射 */
int32_t oal_pcie_ringbuf_res_map(oal_pcie_res *pst_pci_res)
{
    int32_t ret;
    int32_t i;
    uint8_t reg = 0;
    oal_pci_dev_stru *pst_pci_dev;
    pci_addr_map st_map; /* DEVICE CPU地址 */
    pcie_share_mem_stru st_share_mem;

    pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    oal_pci_read_config_byte(pst_pci_dev, PCI_CACHE_LINE_SIZE, &reg);
    pci_print_log(PCI_LOG_INFO, "L1_CACHE_BYTES: %d\n", reg);

    pci_print_log(PCI_LOG_INFO, "pst_pci_res->dev_share_mem.va:%lx", pst_pci_res->dev_share_mem.va);

    oal_pcie_io_trans((uintptr_t)&st_share_mem, pst_pci_res->dev_share_mem.va, sizeof(pcie_share_mem_stru));
    oal_print_hex_dump((uint8_t *)pst_pci_res->dev_share_mem.va, sizeof(pcie_share_mem_stru),
                       HEX_DUMP_GROUP_SIZE, "st_share_mem: ");
    pci_print_log(PCI_LOG_INFO, "st_share_mem.ringbuf_res_paddr :0x%x\n", st_share_mem.ringbuf_res_paddr);

    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, st_share_mem.ringbuf_res_paddr, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "invalid ringbuf device address 0x%x, map failed\n",
                      st_share_mem.ringbuf_res_paddr);
        oal_print_hex_dump((uint8_t *)&st_share_mem, sizeof(st_share_mem),
                           HEX_DUMP_GROUP_SIZE, "st_share_mem: ");
        return -OAL_ENOMEM;
    }

    /* h->h */
    memcpy_s(&pst_pci_res->st_ringbuf_map, sizeof(pci_addr_map), &st_map, sizeof(pci_addr_map));

    /* device的ringbuf管理结构同步到Host */
    oal_pcie_io_trans((uintptr_t)&pst_pci_res->st_ringbuf, pst_pci_res->st_ringbuf_map.va,
                      sizeof(pst_pci_res->st_ringbuf));

    /* 初始化ringbuf 管理结构体的映射 */
    pst_pci_res->st_rx_res.ringbuf_ctrl_dma_addr.pa = pst_pci_res->st_ringbuf_map.pa +
                                                      OAL_OFFSET_OF(pcie_ringbuf_res, st_d2h_buf);
    pst_pci_res->st_rx_res.ringbuf_ctrl_dma_addr.va = pst_pci_res->st_ringbuf_map.va +
                                                      OAL_OFFSET_OF(pcie_ringbuf_res, st_d2h_buf);

    /* 初始化TX BUFF, 不考虑大小端，host/dev 都是小端，否者这里的base_addr需要转换 */
    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, pst_pci_res->st_ringbuf.st_d2h_buf.base_addr, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "invalid h2d ringbuf base address 0x%llx, map failed\n",
                      pst_pci_res->st_ringbuf.st_d2h_buf.base_addr);
        return -OAL_ENOMEM;
    }

    memcpy_s((void *)&pst_pci_res->st_rx_res.ringbuf_data_dma_addr, sizeof(pci_addr_map),
             (void *)&st_map, sizeof(pci_addr_map));

    /* 初始化RX BUFF */
    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        uintptr_t offset;
        ret = oal_pcie_inbound_ca_to_va(pst_pci_res, pst_pci_res->st_ringbuf.st_h2d_buf[i].base_addr, &st_map);
        if (ret != OAL_SUCC) {
            pci_print_log(PCI_LOG_ERR, "invalid d2h ringbuf[%d] base address 0x%llx, map failed, ret=%d\n",
                          i, pst_pci_res->st_ringbuf.st_h2d_buf[i].base_addr, ret);
            return -OAL_ENOMEM;
        }
        memcpy_s(&pst_pci_res->st_tx_res[i].ringbuf_data_dma_addr,
                 sizeof(pci_addr_map), &st_map, sizeof(pci_addr_map));
        offset = (uintptr_t)&pst_pci_res->st_ringbuf.st_h2d_buf[i] - (uintptr_t)&pst_pci_res->st_ringbuf;
        pst_pci_res->st_tx_res[i].ringbuf_ctrl_dma_addr.pa = pst_pci_res->st_ringbuf_map.pa + offset;
        pst_pci_res->st_tx_res[i].ringbuf_ctrl_dma_addr.va = pst_pci_res->st_ringbuf_map.va + offset;
    }

    /* 初始化消息TX RINGBUFF */
    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, pst_pci_res->st_ringbuf.st_h2d_msg.base_addr, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "invalid h2d message ringbuf base address 0x%llx, map failed, ret=%d\n",
                      pst_pci_res->st_ringbuf.st_h2d_msg.base_addr, ret);
        return -OAL_ENOMEM;
    }

    /* h2d message data 所在DMA地址 */
    memcpy_s((void *)&pst_pci_res->st_message_res.h2d_res.ringbuf_data_dma_addr,
             sizeof(pci_addr_map), (void *)&st_map, sizeof(pci_addr_map));

    /* h2d message ctrl 结构体 所在DMA地址 */
    pst_pci_res->st_message_res.h2d_res.ringbuf_ctrl_dma_addr.va = pst_pci_res->st_ringbuf_map.va +
                                                                   OAL_OFFSET_OF(pcie_ringbuf_res, st_h2d_msg);
    pst_pci_res->st_message_res.h2d_res.ringbuf_ctrl_dma_addr.pa = pst_pci_res->st_ringbuf_map.pa +
                                                                   OAL_OFFSET_OF(pcie_ringbuf_res, st_h2d_msg);

    /* 初始化消息RX RINGBUFF */
    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, pst_pci_res->st_ringbuf.st_d2h_msg.base_addr, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "invalid d2h message ringbuf base address 0x%llx, map failed, ret=%d\n",
                      pst_pci_res->st_ringbuf.st_d2h_msg.base_addr, ret);
        return -OAL_ENOMEM;
    }

    /* d2h message data 所在DMA地址 */
    memcpy_s((void *)&pst_pci_res->st_message_res.d2h_res.ringbuf_data_dma_addr,
             sizeof(pci_addr_map), (void *)&st_map, sizeof(pci_addr_map));

    /* d2h message ctrl 结构体 所在DMA地址 */
    pst_pci_res->st_message_res.d2h_res.ringbuf_ctrl_dma_addr.va = pst_pci_res->st_ringbuf_map.va +
                                                                   OAL_OFFSET_OF(pcie_ringbuf_res, st_d2h_msg);
    pst_pci_res->st_message_res.d2h_res.ringbuf_ctrl_dma_addr.pa = pst_pci_res->st_ringbuf_map.pa +
                                                                   OAL_OFFSET_OF(pcie_ringbuf_res, st_d2h_msg);

#ifdef _PRE_PLAT_FEATURE_PCIE_DEVICE_STAT
    ret = oal_pcie_inbound_ca_to_va(pst_pci_res, st_share_mem.device_stat_paddr, &st_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "invalid device_stat_paddr  0x%x, map failed\n", st_share_mem.device_stat_paddr);
        return -OAL_ENOMEM;
    }

    /* h->h */
    memcpy_s(&pst_pci_res->st_device_stat_map, sizeof(pci_addr_map), &st_map, sizeof(pci_addr_map));
    oal_pcie_io_trans((uintptr_t)&pst_pci_res->st_device_stat, pst_pci_res->st_device_stat_map.va,
                      sizeof(pst_pci_res->st_device_stat));
#endif

    ret = oal_pcie_comm_ringbuf_res_map(pst_pci_res);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = oal_pcie_device_shared_addr_res_map(pst_pci_res, &st_share_mem);
    if (ret != OAL_SUCC) {
        return ret;
    }

    pci_print_log(PCI_LOG_INFO, "oal_pcie_ringbuf_res_map succ");
    return OAL_SUCC;
}

int32_t oal_pcie_h2d_doorbell(oal_pcie_res *pst_pci_res)
{
    /* 敲铃,host->device ringbuf 有数据更新,2个队列共享一个中断 */
    pst_pci_res->stat.h2d_doorbell_cnt++;
    pci_print_log(PCI_LOG_DBG, "oal_pcie_h2d_doorbell,cnt:%u", pst_pci_res->stat.h2d_doorbell_cnt);
    if (oal_unlikely(pst_pci_res->link_state <= PCI_WLAN_LINK_DOWN)) {
        pci_print_log(PCI_LOG_WARN, "pcie is linkdown");
        return -OAL_ENODEV;
    }
    oal_writel(PCIE_H2D_DOORBELL_TRIGGER_VALUE, pst_pci_res->pst_pci_ctrl_base + PCIE_H2D_DOORBELL_OFF);
    return OAL_SUCC;
}

int32_t oal_pcie_d2h_doorbell(oal_pcie_res *pst_pci_res)
{
    /* 敲铃,host->device ringbuf 有数据更新,2个队列共享一个中断 */
    pst_pci_res->stat.d2h_doorbell_cnt++;
    pci_print_log(PCI_LOG_DBG, "oal_pcie_d2h_doorbell,cnt:%u", pst_pci_res->stat.d2h_doorbell_cnt);
    if (oal_unlikely(pst_pci_res->link_state <= PCI_WLAN_LINK_DOWN)) {
        pci_print_log(PCI_LOG_WARN, "pcie is linkdown");
        return -OAL_ENODEV;
    }
    oal_writel(PCIE_D2H_DOORBELL_TRIGGER_VALUE, pst_pci_res->pst_pci_ctrl_base + PCIE_D2H_DOORBELL_OFF);
    return OAL_SUCC;
}

/* write message to ringbuf */
/* Returns, bytes we wrote to ringbuf */
OAL_STATIC int32_t oal_pcie_h2d_message_buf_write(oal_pcie_res *pst_pci_res, pcie_ringbuf *pst_ringbuf,
                                                  pci_addr_map *pst_ringbuf_base, uint32_t message)
{
    uint32_t real_wr;

    if (oal_unlikely(pst_ringbuf->item_len != (uint16_t)sizeof(message))) {
        pci_print_log(PCI_LOG_ERR, "[%s]invalid item_len [%u!=%lu]\n",
                      __FUNCTION__, pst_ringbuf->item_len, (unsigned long)sizeof(pcie_read_ringbuf_item));
        return 0;
    }

    if (oal_warn_on(pst_ringbuf->wr - pst_ringbuf->rd >= pst_ringbuf->size)) {
        /* never touch here */
        pci_print_log(PCI_LOG_ERR, "message ringbuf full [wr:%u] [rd:%u] [size:%u]\n",
                      pst_ringbuf->wr, pst_ringbuf->rd, pst_ringbuf->size);
        return 0;
    }

    real_wr = pst_ringbuf->wr & (pst_ringbuf->size - 1);
    oal_pcie_io_trans(pst_ringbuf_base->va + real_wr, (uintptr_t)&message, sizeof(message));

    pst_ringbuf->wr += sizeof(message);

    return 1;
}

OAL_STATIC int32_t oal_pcie_h2d_message_buf_rd_update(oal_pcie_res *pst_pci_res)
{
    /* 需要刷cache */
    /* h2d方向，同步device的读指针到HOST message ringbuf管理结构体 */
    uint32_t rd;
    pci_addr_map st_map;

    st_map.va = pst_pci_res->st_message_res.h2d_res.ringbuf_ctrl_dma_addr.va + OAL_OFFSET_OF(pcie_ringbuf, rd);
    st_map.pa = pst_pci_res->st_message_res.h2d_res.ringbuf_ctrl_dma_addr.pa + OAL_OFFSET_OF(pcie_ringbuf, rd);

    rd = oal_pcie_read_mem32(st_map.va);
    if (oal_unlikely(rd == 0xFFFFFFFF)) {
        if (oal_pcie_check_link_state(pst_pci_res) == OAL_FALSE) {
            pci_print_log(PCI_LOG_ERR, "h2d message ringbuf rd update: link down[va:0x%lx, pa:0x%lx]",
                          st_map.va, st_map.pa);
            return -OAL_ENODEV;
        }
    }
    pci_print_log(PCI_LOG_DBG, "h2d message ringbuf rd update:[0x%lx:rd:0x%x]", st_map.va, rd);
    pst_pci_res->st_ringbuf.st_h2d_msg.rd = rd;

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_h2d_message_buf_wr_update(oal_pcie_res *pst_pci_res)
{
    /* 需要刷cache */
    /* h2d方向，同步device的读指针到HOST message ringbuf管理结构体 */
    uint32_t wr_back;
    pci_addr_map st_map;

    st_map.va = pst_pci_res->st_message_res.h2d_res.ringbuf_ctrl_dma_addr.va + OAL_OFFSET_OF(pcie_ringbuf, wr);
    st_map.pa = pst_pci_res->st_message_res.h2d_res.ringbuf_ctrl_dma_addr.pa + OAL_OFFSET_OF(pcie_ringbuf, wr);

    oal_pcie_write_mem32(st_map.va, pst_pci_res->st_ringbuf.st_h2d_msg.wr);

    wr_back = oal_pcie_read_mem32(st_map.va);
    if (wr_back != pst_pci_res->st_ringbuf.st_h2d_msg.wr) {
        pci_print_log(PCI_LOG_ERR, "pcie h2d message wr write failed, wr_back=%u, host_wr=%u",
                      wr_back, pst_pci_res->st_ringbuf.st_h2d_msg.wr);
        declare_dft_trace_key_info("h2d_message_wr_update_failed", OAL_DFT_TRACE_FAIL);
    }

    return OAL_SUCC;
}

/* update wr pointer to host ,check the read space */
OAL_STATIC int32_t oal_pcie_d2h_message_buf_wr_update(oal_pcie_res *pst_pci_res)
{
    uint32_t wr;
    pci_addr_map st_map;

    st_map.va = pst_pci_res->st_message_res.d2h_res.ringbuf_ctrl_dma_addr.va + OAL_OFFSET_OF(pcie_ringbuf, wr);
    st_map.pa = pst_pci_res->st_message_res.d2h_res.ringbuf_ctrl_dma_addr.pa + OAL_OFFSET_OF(pcie_ringbuf, wr);

    wr = oal_pcie_read_mem32(st_map.va);
    if (oal_unlikely(wr == 0xFFFFFFFF)) {
        if (oal_pcie_check_link_state(pst_pci_res) == OAL_FALSE) {
            pci_print_log(PCI_LOG_ERR, "d2h message ringbuf wr update: link down[va:0x%lx, pa:0x%lx]",
                          st_map.va, st_map.pa);
            return -OAL_ENODEV;
        }
    }

    pci_print_log(PCI_LOG_DBG, "d2h message ringbuf wr update:[0x%lx:wr:0x%x]", st_map.va, wr);
    pst_pci_res->st_ringbuf.st_d2h_msg.wr = wr;
    return OAL_SUCC;
}

/* update rd pointer to device */
OAL_STATIC int32_t oal_pcie_d2h_message_buf_rd_update(oal_pcie_res *pst_pci_res)
{
    pci_addr_map st_map;

    st_map.va = pst_pci_res->st_message_res.d2h_res.ringbuf_ctrl_dma_addr.va + OAL_OFFSET_OF(pcie_ringbuf, rd);
    st_map.pa = pst_pci_res->st_message_res.d2h_res.ringbuf_ctrl_dma_addr.pa + OAL_OFFSET_OF(pcie_ringbuf, rd);

    oal_pcie_write_mem32(st_map.va, pst_pci_res->st_ringbuf.st_d2h_msg.rd);

    return OAL_SUCC;
}

/* Update rd pointer, Return the bytes we read */
OAL_STATIC int32_t oal_pcie_d2h_message_buf_read(oal_pcie_res *pst_pci_res, pcie_ringbuf *pst_ringbuf,
                                                 pci_addr_map *pst_ringbuf_base, uint32_t *message)
{
    uint32_t real_rd, wr, rd, data_size;

    rd = pst_ringbuf->rd;
    wr = pst_ringbuf->wr;

    data_size = wr - rd;

    if (oal_unlikely((data_size < pst_ringbuf->item_len) || (pst_ringbuf->item_len != sizeof(uint32_t)))) {
        pci_print_log(PCI_LOG_ERR, "d2h message buf read failed, date_size[%d] < item_len:%d, wr:%u, rd:%u",
                      data_size, pst_ringbuf->item_len, wr, rd);
        return 0;
    }

    real_rd = rd & (pst_ringbuf->size - 1);

    if (oal_pcie_check_link_state(pst_pci_res) == OAL_FALSE) {
        /* LinkDown */
        pci_print_log(PCI_LOG_ERR, "d2h message read detect linkdown.");
        return 0;
    }

    oal_pcie_io_trans((uintptr_t)(message), (uintptr_t)(pst_ringbuf_base->va + (unsigned long)real_rd),
                      pst_ringbuf->item_len);

    pst_ringbuf->rd += pst_ringbuf->item_len;

    /* Update device's read pointer */
    oal_pcie_d2h_message_buf_rd_update(pst_pci_res);

    return pst_ringbuf->item_len;
}

int32_t oal_pcie_ringbuf_read_rd(oal_pcie_res *pst_pci_res, pcie_comm_ringbuf_type type)
{
    uint32_t rd;
    pci_addr_map st_map;
    pcie_ringbuf *pst_ringbuf = NULL;

    if (oal_unlikely(pst_pci_res->link_state < PCI_WLAN_LINK_RES_UP)) {
        pci_print_log(PCI_LOG_WARN, "comm ringbuf %d read rd, link_state:%s",
                      type, oal_pcie_get_link_state_str(pst_pci_res->link_state));
        return -OAL_ENODEV;
    }

    pst_ringbuf = &pst_pci_res->st_ringbuf.st_ringbuf[type];

    st_map.va = pst_pci_res->st_ringbuf_res.comm_rb_res[type].ctrl_daddr.va + OAL_OFFSET_OF(pcie_ringbuf, rd);
    st_map.pa = pst_pci_res->st_ringbuf_res.comm_rb_res[type].ctrl_daddr.pa + OAL_OFFSET_OF(pcie_ringbuf, rd);

    rd = oal_pcie_read_mem32(st_map.va);
    if (oal_unlikely(rd == 0xFFFFFFFF)) {
        if (oal_pcie_check_link_state(pst_pci_res) == OAL_FALSE) {
            pci_print_log(PCI_LOG_ERR, "ringbuf %d read rd: link down[va:0x%lx, pa:0x%lx]",
                          type, st_map.va, st_map.pa);
            return -OAL_ENODEV;
        }
    }

    pci_print_log(PCI_LOG_DBG, "ringbuf %d read rd:[0x%lx:rd:0x%x]", type, st_map.va, rd);

    pst_ringbuf->rd = rd;

    return OAL_SUCC;
}

int32_t oal_pcie_ringbuf_write(oal_pcie_res *pst_pci_res,
                               pcie_comm_ringbuf_type type, uint8_t *buf, uint32_t len)
{
    /* 不判断写指针，此函数只执行写操作 */
    oal_pci_dev_stru *pst_pci_dev;
    uint32_t real_wr;

    pcie_ringbuf *pst_ringbuf = &pst_pci_res->st_ringbuf.st_ringbuf[type];
    pci_addr_map *pst_ringbuf_base = &pst_pci_res->st_ringbuf_res.comm_rb_res[type].data_daddr;

    pst_pci_dev = pcie_res_to_dev(pst_pci_res);

    /* Debug */
    if (oal_unlikely(pst_pci_res->link_state < PCI_WLAN_LINK_RES_UP)) {
        pci_print_log(PCI_LOG_WARN, "comm ringbuf %d write failed, link_state:%s",
                      type, oal_pcie_get_link_state_str(pst_pci_res->link_state));
        return 0;
    }

    if (oal_warn_on(len != pst_ringbuf->item_len)) {
        pci_print_log(PCI_LOG_WARN, "ringbuf %d write request len %u not equal to %u",
                      type, len, pst_ringbuf->item_len);
        return 0;
    }

    if (oal_warn_on(pst_ringbuf->wr - pst_ringbuf->rd >= pst_ringbuf->size)) {
        /* never touch here */
        pci_print_log(PCI_LOG_ERR, "ringbuf %d full [wr:%u] [rd:%u] [size:%u]\n",
                      type, pst_ringbuf->wr, pst_ringbuf->rd, pst_ringbuf->size);
        return 0;
    }

    real_wr = pst_ringbuf->wr & (pst_ringbuf->size - 1);
    oal_pcie_io_trans(pst_ringbuf_base->va + real_wr, (uintptr_t)buf, pst_ringbuf->item_len);
    if (pci_dbg_condtion()) {
        int32_t ret;
        uint64_t cpuaddr;
        ret = oal_pcie_get_ca_by_pa(pst_pci_res, pst_ringbuf_base->pa, &cpuaddr);
        if (ret == OAL_SUCC) {
            pci_print_log(PCI_LOG_DBG, "ringbuf %d write ringbuf data cpu address:0x%llx", type, cpuaddr);
        } else {
            pci_print_log(PCI_LOG_DBG, "ringbuf %d rd pa:0x%lx invaild", type, pst_ringbuf_base->pa);
        }
        oal_print_hex_dump((uint8_t *)buf, pst_ringbuf->item_len, pst_ringbuf->item_len, "ringbuf write: ");
    }

    pst_ringbuf->wr += pst_ringbuf->item_len;

    return 1;
}

uint32_t oal_pcie_comm_ringbuf_freecount(oal_pcie_res *pst_pci_res,
                                         pcie_comm_ringbuf_type type)
{
    pcie_ringbuf *pst_ringbuf = &pst_pci_res->st_ringbuf.st_ringbuf[type];
    return oal_pcie_ringbuf_freecount(pst_ringbuf);
}

int32_t oal_pcie_read_d2h_message(oal_pcie_res *pst_pci_res, uint32_t *message)
{
    int32_t ret;
    uint32_t len;
    pcie_ringbuf *pst_ringbuf = NULL;
    if (oal_unlikely(pst_pci_res == NULL)) {
        return -OAL_EINVAL;
    }

    if (oal_unlikely(pst_pci_res->link_state < PCI_WLAN_LINK_RES_UP)) {
        pci_print_log(PCI_LOG_INFO, "link state is disabled:%s!",
                      oal_pcie_get_link_state_str(pst_pci_res->link_state));
        return -OAL_ENODEV;
    }

    pst_ringbuf = &pst_pci_res->st_ringbuf.st_d2h_msg;
    pci_print_log(PCI_LOG_DBG, "oal_pcie_read_d2h_message ++");

    len = pcie_ringbuf_len(pst_ringbuf);
    if (len == 0) {
        /* No Message, update wr pointer and retry */
        ret = oal_pcie_d2h_message_buf_wr_update(pst_pci_res);
        if (ret != OAL_SUCC) {
            return ret;
        }
        len = pcie_ringbuf_len(&pst_pci_res->st_ringbuf.st_d2h_msg);
    }

    if (len == 0) {
        return -OAL_ENODEV;
    }

    if (oal_pcie_d2h_message_buf_read(pst_pci_res, pst_ringbuf,
                                      &pst_pci_res->st_message_res.d2h_res.ringbuf_data_dma_addr, message)) {
        pci_print_log(PCI_LOG_DBG, "oal_pcie_read_d2h_message --");
        return OAL_SUCC;
    } else {
        pci_print_log(PCI_LOG_DBG, "oal_pcie_read_d2h_message ^^");
        return -OAL_EINVAL;
    }
}

void oal_pcie_trigger_message(oal_pcie_res *pst_pci_res)
{
    if (pst_pci_res->chip_info.edma_support == OAL_TRUE) {
        /* 触发h2d int */
        oal_writel(PCIE_H2D_TRIGGER_VALUE, pst_pci_res->pst_pci_ctrl_base + PCIE_D2H_DOORBELL_OFF);
    } else if (pst_pci_res->chip_info.ete_support == OAL_TRUE) {
        oal_pcie_h2d_int(pst_pci_res);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "unkown dma");
    }
}

int32_t oal_pcie_send_message_to_dev(oal_pcie_res *pst_pci_res, uint32_t message)
{
    int32_t ret;
    uint32_t freecount;
    if (oal_warn_on(pst_pci_res == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_unlikely(!pst_pci_res->regions.inited)) {
        pci_print_log(PCI_LOG_ERR, "region is disabled!");
        return -OAL_EFAUL;
    }

    if (oal_unlikely(pst_pci_res->link_state < PCI_WLAN_LINK_RES_UP)) {
        pci_print_log(PCI_LOG_WARN, "send message link invaild, link_state:%s",
                      oal_pcie_get_link_state_str(pst_pci_res->link_state));
        return -OAL_EBUSY;
    }

    /* message ringbuf freecount */
    oal_spin_lock(&pst_pci_res->st_message_res.h2d_res.lock);
    freecount = oal_pcie_ringbuf_freecount(&pst_pci_res->st_ringbuf.st_h2d_msg);
    if (freecount == 0) {
        /* no space, sync rd pointer */
        oal_pcie_h2d_message_buf_rd_update(pst_pci_res);
        freecount = oal_pcie_ringbuf_freecount(&pst_pci_res->st_ringbuf.st_h2d_msg);
    }

    if (freecount == 0) {
        oal_spin_unlock(&pst_pci_res->st_message_res.h2d_res.lock);
        return -OAL_EBUSY;
    }

    /* write message to ringbuf */
    ret = oal_pcie_h2d_message_buf_write(pst_pci_res, &pst_pci_res->st_ringbuf.st_h2d_msg,
                                         &pst_pci_res->st_message_res.h2d_res.ringbuf_data_dma_addr,
                                         message);
    if (ret <= 0) {
        oal_spin_unlock(&pst_pci_res->st_message_res.h2d_res.lock);
        if (oal_pcie_check_link_state(pst_pci_res) == OAL_FALSE) {
            /* Should trigger DFR here */
            pci_print_log(PCI_LOG_ERR, "h2d message send failed: link down, ret=%d", ret);
        }
        return -OAL_EIO;
    }

    /* 更新写指针 */
    oal_pcie_h2d_message_buf_wr_update(pst_pci_res);

    /* 触发h2d int */
    oal_pcie_trigger_message(pst_pci_res);

    oal_spin_unlock(&pst_pci_res->st_message_res.h2d_res.lock);

    return OAL_SUCC;
}

int32_t oal_pcie_get_host_trans_count(oal_pcie_res *pst_pci_res, uint64_t *tx, uint64_t *rx)
{
    if (tx != NULL) {
        int32_t i;
        *tx = 0;
        for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
            *tx += (uint64_t)pst_pci_res->st_tx_res[i].stat.tx_count;
        }
    }
    if (rx != NULL) {
        *rx = (uint64_t)pst_pci_res->st_rx_res.stat.rx_count;
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_print_pcie_regs(oal_pcie_res *pst_pci_res, uint32_t base, uint32_t size)
{
    int32_t i;
    uint32_t value;
    void *pst_mem = NULL;
    pci_addr_map addr_map;
    size = OAL_ROUND_UP(size, 4); /* 计算4字节对齐后的长度，默认进位 */
    if ((size == 0) || (size > 0xffff)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "size invalid %u\n", size);
        return -OAL_EFAIL;
    }

    if (oal_pcie_inbound_ca_to_va(pst_pci_res, base, &addr_map) != OAL_SUCC) {
        /* share mem 地址未映射! */
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "can not found mem map for dev cpu address 0x%x\n", base);
        return -OAL_EFAIL;
    }

    pst_mem = vmalloc(size);
    if (pst_mem == NULL) {
        pci_print_log(PCI_LOG_WARN, "vmalloc mem size %u failed", size);
    } else {
        memset_s(pst_mem, size, 0, size);
    }

    for (i = 0; i < size; i += sizeof(uint32_t)) { /* 每次偏移4字节 */
        value = oal_readl((void *)addr_map.va + i);
        if (value == 0xffffffff) {
            if (oal_pcie_device_check_alive(pst_pci_res) != OAL_SUCC) {
                if (pst_mem != NULL) {
                    vfree(pst_mem);
                }
                return -OAL_ENODEV;
            }
        }

        if (pst_mem != NULL) {
            oal_writel(value, pst_mem + i);
        } else {
            oal_io_print("%8x:%8x\n", base + i, value);
        }
    }

    if (pst_mem != NULL) {
        if (i) {
            pci_print_log(PCI_LOG_INFO, "dump regs base 0x%x", base);
#ifdef CONFIG_PRINTK
            /* print to kenrel msg, 32B per */
            print_hex_dump(KERN_INFO, "pcie regs: ", DUMP_PREFIX_OFFSET, 32, 4,
                           pst_mem, i, false); /* 内核函数固定的传参 */
#endif
        }

        vfree(pst_mem);
    }

    return OAL_SUCC;
}

int32_t oal_pcie_print_device_transfer_info(oal_pcie_res *pst_pcie_res)
{
    int32_t ret;
    if (pst_pcie_res->st_device_stat_map.va != 0) {
        ret = oal_pcie_device_check_alive(pst_pcie_res);
        if (ret != OAL_SUCC) {
            return ret;
        }

        pci_print_log(PCI_LOG_INFO, "show device info:");

        oal_pcie_io_trans((uintptr_t)&pst_pcie_res->st_device_stat,
                          pst_pcie_res->st_device_stat_map.va,
                          sizeof(pst_pcie_res->st_device_stat));

        pci_print_log(PCI_LOG_INFO, "d2h fifo_full:%u, fifo_notfull:%u ringbuf_hit:%u, ringbuf_miss:%u\n",
                      pst_pcie_res->st_device_stat.d2h_stats.stat.fifo_full,
                      pst_pcie_res->st_device_stat.d2h_stats.stat.fifo_notfull,
                      pst_pcie_res->st_device_stat.d2h_stats.stat.ringbuf_hit,
                      pst_pcie_res->st_device_stat.d2h_stats.stat.ringbuf_miss);
        pci_print_log(PCI_LOG_INFO, "d2h dma_busy:%u, dma_idle:%u fifo_ele_empty:%u doorbell count:%u\n",
                      pst_pcie_res->st_device_stat.d2h_stats.stat.fifo_dma_busy,
                      pst_pcie_res->st_device_stat.d2h_stats.stat.fifo_dma_idle,
                      pst_pcie_res->st_device_stat.d2h_stats.stat.fifo_ele_empty,
                      pst_pcie_res->st_device_stat.d2h_stats.stat.doorbell_isr_count);

        pci_print_log(PCI_LOG_INFO,
                      "d2h push_fifo_count:%u done_isr_count:%u dma_work_list_stat:%u "
                      "dma_free_list_stat:%u dma_pending_list_stat:%u",
                      pst_pcie_res->st_device_stat.d2h_stats.stat.push_fifo_count,
                      pst_pcie_res->st_device_stat.d2h_stats.stat.done_isr_count,
                      pst_pcie_res->st_device_stat.d2h_stats.stat.dma_work_list_stat,
                      pst_pcie_res->st_device_stat.d2h_stats.stat.dma_free_list_stat,
                      pst_pcie_res->st_device_stat.d2h_stats.stat.dma_pending_list_stat);

        pci_print_log(PCI_LOG_INFO,
                      "h2d fifo_full:%u, fifo_notfull:%u ringbuf_hit:%u, ringbuf_miss:%u fifo_ele_empty:%u\n",
                      pst_pcie_res->st_device_stat.h2d_stats.stat.fifo_full,
                      pst_pcie_res->st_device_stat.h2d_stats.stat.fifo_notfull,
                      pst_pcie_res->st_device_stat.h2d_stats.stat.ringbuf_hit,
                      pst_pcie_res->st_device_stat.h2d_stats.stat.ringbuf_miss,
                      pst_pcie_res->st_device_stat.h2d_stats.stat.fifo_ele_empty);

        pci_print_log(PCI_LOG_INFO,
                      "h2d push_fifo_count:%u done_isr_count:%u dma_work_list_stat:%u dma_free_list_stat:%u "
                      "dma_pending_list_stat:%u",
                      pst_pcie_res->st_device_stat.h2d_stats.stat.push_fifo_count,
                      pst_pcie_res->st_device_stat.h2d_stats.stat.done_isr_count,
                      pst_pcie_res->st_device_stat.h2d_stats.stat.dma_work_list_stat,
                      pst_pcie_res->st_device_stat.h2d_stats.stat.dma_free_list_stat,
                      pst_pcie_res->st_device_stat.h2d_stats.stat.dma_pending_list_stat);

        pci_print_log(PCI_LOG_INFO,
                      "comm_stat l1_wake_force_push_cnt:%u l1_wake_l1_hit:%u l1_wake_l1_miss:%u "
                      "l1_wake_state_err_cnt:%u l1_wake_timeout_cnt:%u l1_wake_timeout_max_cnt:%u",
                      pst_pcie_res->st_device_stat.comm_stat.l1_wake_force_push_cnt,
                      pst_pcie_res->st_device_stat.comm_stat.l1_wake_l1_hit,
                      pst_pcie_res->st_device_stat.comm_stat.l1_wake_l1_miss,
                      pst_pcie_res->st_device_stat.comm_stat.l1_wake_state_err_cnt,
                      pst_pcie_res->st_device_stat.comm_stat.l1_wake_timeout_cnt,
                      pst_pcie_res->st_device_stat.comm_stat.l1_wake_timeout_max_cnt);
        if (pst_pcie_res->st_device_stat.comm_stat.l1_wake_force_push_cnt) {
            declare_dft_trace_key_info("l1_wake_force_push_error", OAL_DFT_TRACE_FAIL);
        }
        if (pst_pcie_res->st_device_stat.comm_stat.l1_wake_state_err_cnt) {
            declare_dft_trace_key_info("l1_wake_state_err", OAL_DFT_TRACE_FAIL);
        }
    } else {
        pci_print_log(PCI_LOG_INFO, "st_device_stat_map null:va:%lu, pa:%lu\n",
                      pst_pcie_res->st_device_stat_map.va, pst_pcie_res->st_device_stat_map.pa);
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_print_all_ringbuf_info(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    oal_pcie_print_ringbuf_info(&pst_pci_res->st_ringbuf.st_d2h_buf, PCI_LOG_INFO);
    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        oal_pcie_print_ringbuf_info(&pst_pci_res->st_ringbuf.st_h2d_buf[i], PCI_LOG_INFO);
    }
    oal_pcie_print_ringbuf_info(&pst_pci_res->st_ringbuf.st_d2h_msg, PCI_LOG_INFO);
    oal_pcie_print_ringbuf_info(&pst_pci_res->st_ringbuf.st_h2d_msg, PCI_LOG_INFO);

    /* soft ringbuf */
    return OAL_SUCC;
}

static void oal_pcie_print_comm_info(oal_pcie_res *pst_pci_res)
{
    if (pst_pci_res->stat.intx_total_count) {
        pci_print_log(PCI_LOG_INFO, "intx_total_count:%u", pst_pci_res->stat.intx_total_count);
    }
    if (pst_pci_res->stat.intx_tx_count) {
        pci_print_log(PCI_LOG_INFO, "intx_tx_count:%u", pst_pci_res->stat.intx_tx_count);
    }
    if (pst_pci_res->stat.intx_rx_count) {
        pci_print_log(PCI_LOG_INFO, "intx_rx_count:%u", pst_pci_res->stat.intx_rx_count);
    }
    if (pst_pci_res->stat.done_err_cnt) {
        pci_print_log(PCI_LOG_INFO, "done_err_cnt:%u", pst_pci_res->stat.done_err_cnt);
    }
    if (pst_pci_res->stat.h2d_doorbell_cnt) {
        pci_print_log(PCI_LOG_INFO, "h2d_doorbell_cnt:%u", pst_pci_res->stat.h2d_doorbell_cnt);
    }
    if (pst_pci_res->stat.d2h_doorbell_cnt) {
        pci_print_log(PCI_LOG_INFO, "d2h_doorbell_cnt:%u", pst_pci_res->stat.d2h_doorbell_cnt);
    }
    if (pst_pci_res->st_rx_res.stat.rx_count) {
        pci_print_log(PCI_LOG_INFO, "rx_count:%u", pst_pci_res->st_rx_res.stat.rx_count);
    }
    if (pst_pci_res->st_rx_res.stat.rx_done_count) {
        pci_print_log(PCI_LOG_INFO, "rx_done_count:%u", pst_pci_res->st_rx_res.stat.rx_done_count);
    }
    if (pst_pci_res->st_rx_res.stat.alloc_netbuf_failed) {
        pci_print_log(PCI_LOG_INFO, "alloc_netbuf_failed:%u", pst_pci_res->st_rx_res.stat.alloc_netbuf_failed);
    }
    if (pst_pci_res->st_rx_res.stat.map_netbuf_failed) {
        pci_print_log(PCI_LOG_INFO, "map_netbuf_failed:%u", pst_pci_res->st_rx_res.stat.map_netbuf_failed);
    }
}

static void oal_pcie_print_transfer_stat_info(oal_pcie_res *pst_pci_res)
{
    int32_t i;
    int32_t j;
    uint32_t len;
    uint32_t total_len = 0;

    /* tx info */
    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        if (pst_pci_res->st_tx_res[i].stat.tx_count) {
            pci_print_log(PCI_LOG_INFO, "[qid:%d]tx_count:%u", i, pst_pci_res->st_tx_res[i].stat.tx_count);
        }
        if (pst_pci_res->st_tx_res[i].stat.tx_done_count) {
            pci_print_log(PCI_LOG_INFO, "[qid:%d]tx_done_count:%u", i,
                          pst_pci_res->st_tx_res[i].stat.tx_done_count);
        }
        len = oal_netbuf_list_len(&pst_pci_res->st_tx_res[i].txq);
        if (len) {
            pci_print_log(PCI_LOG_INFO, "[qid:%d]len=%d", i, len);
            total_len += len;
        }

        pci_print_log(PCI_LOG_INFO, "[qid:%d]tx ringbuf cond is %d",
                      i, oal_atomic_read(&pst_pci_res->st_tx_res[i].tx_ringbuf_sync_cond));
    }

    /* burst info */
    for (i = 0; i < PCIE_EDMA_WRITE_BUSRT_COUNT + 1; i++) {
        if (pst_pci_res->st_rx_res.stat.rx_burst_cnt[i]) {
            pci_print_log(PCI_LOG_INFO, "rx burst %d count:%u", i, pst_pci_res->st_rx_res.stat.rx_burst_cnt[i]);
        }
    }

    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        for (j = 0; j < PCIE_EDMA_READ_BUSRT_COUNT + 1; j++) {
            if (pst_pci_res->st_tx_res[i].stat.tx_burst_cnt[j]) {
                pci_print_log(PCI_LOG_INFO, "tx qid %d burst %d count:%u",
                              i, j, pst_pci_res->st_tx_res[i].stat.tx_burst_cnt[j]);
            }
        }
    }
}

void oal_pcie_print_transfer_info(oal_pcie_res *pst_pci_res, uint64_t print_flag)
{
    if (pst_pci_res == NULL) {
        return;
    }

    pci_print_log(PCI_LOG_INFO, "pcie transfer info:");
    oal_pcie_print_comm_info(pst_pci_res);
    oal_pcie_print_transfer_stat_info(pst_pci_res);
    oal_pcie_print_all_ringbuf_info(pst_pci_res);

    /* dump pcie hardware info */
    if (oal_unlikely(pst_pci_res->link_state >= PCI_WLAN_LINK_WORK_UP)) {
        if (board_get_host_wakeup_dev_stat() == 1) {
            /* gpio is high axi is alive */
            if (print_flag & HCC_PRINT_TRANS_FLAG_DEVICE_REGS) {
                oal_pcie_print_pcie_regs(pst_pci_res, PCIE_CTRL_BASE_ADDR, 0x4c8 + 0x4);
                oal_pcie_print_pcie_regs(pst_pci_res, PCIE_DMA_CTRL_BASE_ADDR, 0x30 + 0x4);
            }

            if (print_flag & HCC_PRINT_TRANS_FLAG_DEVICE_STAT) {
                /* dump pcie status */
                oal_pcie_print_device_transfer_info(pst_pci_res);
            }
        }
    } else {
        pci_print_log(PCI_LOG_INFO, "pcie is %s", g_pcie_link_state_str[pst_pci_res->link_state]);
    }
}

void oal_pcie_reset_transfer_info(oal_pcie_res *pst_pci_res)
{
    int32_t i;

    if (pst_pci_res == NULL) {
        return;
    }

    pci_print_log(PCI_LOG_INFO, "reset transfer info");
    pst_pci_res->stat.intx_total_count = 0;
    pst_pci_res->stat.intx_tx_count = 0;
    pst_pci_res->stat.intx_rx_count = 0;
    pst_pci_res->stat.h2d_doorbell_cnt = 0;
    pst_pci_res->stat.d2h_doorbell_cnt = 0;
    pst_pci_res->st_rx_res.stat.rx_count = 0;
    pst_pci_res->st_rx_res.stat.rx_done_count = 0;

    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        pst_pci_res->st_tx_res[i].stat.tx_count = 0;
        pst_pci_res->st_tx_res[i].stat.tx_done_count = 0;
        memset_s((void *)pst_pci_res->st_tx_res[i].stat.tx_burst_cnt,
                 sizeof(pst_pci_res->st_tx_res[i].stat.tx_burst_cnt), 0,
                 sizeof(pst_pci_res->st_tx_res[i].stat.tx_burst_cnt));
    }

    memset_s((void *)pst_pci_res->st_rx_res.stat.rx_burst_cnt,
             sizeof(pst_pci_res->st_rx_res.stat.rx_burst_cnt), 0,
             sizeof(pst_pci_res->st_rx_res.stat.rx_burst_cnt));
}

OAL_STATIC int32_t oal_pcie_sysfs_init(oal_pcie_res *pst_pcie_res)
{
    int32_t ret;
    oal_kobject *pst_root_object = NULL;

    pst_root_object = oal_get_sysfs_root_object();
    if (pst_root_object == NULL) {
        pci_print_log(PCI_LOG_ERR, "[E]get pci root sysfs object failed!\n");
        return -OAL_EFAIL;
    }

    g_conn_syfs_pci_object = kobject_create_and_add("pci", pst_root_object);
    if (g_conn_syfs_pci_object == NULL) {
        ret = -OAL_ENODEV;
        pci_print_log(PCI_LOG_ERR, "sysfs create kobject_create_and_add pci fail\n");
        goto fail_g_conn_syfs_pci_object;
    }

    ret = oal_pcie_sysfs_group_create(g_conn_syfs_pci_object);
    if (ret) {
        ret = -OAL_ENOMEM;
        pci_print_log(PCI_LOG_ERR, "sysfs create g_hpci_attribute_group group fail.ret=%d\n", ret);
        goto fail_create_pci_group;
    }

    return OAL_SUCC;
fail_create_pci_group:
    kobject_put(g_conn_syfs_pci_object);
    g_conn_syfs_pci_object = NULL;
fail_g_conn_syfs_pci_object:
    return ret;
}

OAL_STATIC void oal_pcie_sysfs_exit(oal_pcie_res *pst_pcie_res)
{
    if (g_conn_syfs_pci_object == NULL) {
        return;
    }
    oal_pcie_sysfs_group_remove(g_conn_syfs_pci_object);
    kobject_put(g_conn_syfs_pci_object);
    g_conn_syfs_pci_object = NULL;
}

oal_pcie_res *oal_pcie_host_init(void *data, oal_pcie_msi_stru *msi, uint32_t revision)
{
    int32_t i;
    int32_t ret;
    oal_pcie_res *pci_res = NULL;
    if (g_pci_res) {
        pci_print_log(PCI_LOG_ERR, "PCIe host had already init!\n");
        return NULL;
    }

    if (oal_netbuf_cb_size() < sizeof(pcie_cb_dma_res) + sizeof(struct hcc_tx_cb_stru)) {
        pci_print_log(PCI_LOG_ERR, "pcie cb is too large,[cb %lu < pcie cb %lu + hcc cb %lu]\n",
                      (unsigned long)oal_netbuf_cb_size(),
                      (unsigned long)sizeof(pcie_cb_dma_res),
                      (unsigned long)sizeof(struct hcc_tx_cb_stru));
        return NULL;
    }

    pci_res = (oal_pcie_res *)oal_memalloc(sizeof(oal_pcie_res));
    if (pci_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "alloc pcie res failed, size:%u\n", (uint32_t)sizeof(oal_pcie_res));
        return NULL;
    }

    memset_s((void *)pci_res, sizeof(oal_pcie_res), 0, sizeof(oal_pcie_res));
    pci_res->data = data;
    pci_res->revision = revision;

    /* 初始化tx/rx队列 */
    oal_spin_lock_init(&pci_res->st_rx_res.lock);
    oal_netbuf_list_head_init(&pci_res->st_rx_res.rxq);

    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        oal_atomic_set(&pci_res->st_tx_res[i].tx_ringbuf_sync_cond, 0);
        oal_spin_lock_init(&pci_res->st_tx_res[i].lock);
        oal_netbuf_list_head_init(&pci_res->st_tx_res[i].txq);
    }

    oal_spin_lock_init(&pci_res->st_message_res.d2h_res.lock);
    oal_spin_lock_init(&pci_res->st_message_res.h2d_res.lock);

    ret = oal_pcie_sysfs_init(pci_res);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "oal_pcie_sysfs_init failed, ret=%d\n", ret);
        oal_free(pci_res);
        return NULL;
    }

    ret = oal_pcie_chip_info_init(pci_res);
    if (ret != OAL_SUCC) {
        oal_pcie_sysfs_exit(pci_res);
        oal_free(pci_res);
        return NULL;
    }

    mutex_init(&pci_res->st_rx_mem_lock);

    ret = oal_pcie_edma_task_init(pci_res, msi);
    if (ret != OAL_SUCC) {
        oal_pcie_sysfs_exit(pci_res);
        oal_free(pci_res);
        return NULL;
    }

    oal_ete_host_init(pci_res, msi);

    oal_pcie_change_link_state(pci_res, PCI_WLAN_LINK_UP);

    g_pci_res = pci_res;

    return pci_res;
}

void oal_pcie_host_exit(oal_pcie_res *pci_res)
{
    g_pci_res = NULL;
    oal_ete_host_exit(pci_res);
    oal_pcie_edma_task_exit(pci_res);
    mutex_destroy(&pci_res->st_rx_mem_lock);
    oal_pcie_sysfs_exit(pci_res);
    oal_free(pci_res);
}

#endif

