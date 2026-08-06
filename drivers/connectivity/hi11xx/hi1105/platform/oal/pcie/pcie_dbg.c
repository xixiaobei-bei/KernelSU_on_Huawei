

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


/* Debug functions */
OAL_STATIC int32_t oal_pcie_dump_all_regions_mem(struct kobject *dev, struct kobj_attribute *attr,
                                                 const char *buf, oal_pcie_res *pst_pcie_res)
{
    int32_t index;
    uint32_t region_num;
    oal_pcie_region *region_base = NULL;
    oal_pci_dev_stru *pst_pci_dev = NULL;

    /* 1181 test , get pci cfg memory */
    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_WARN, "pci not init!\n");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        return -OAL_ENODEV;
    }

    if (oal_warn_on(!pst_pcie_res->regions.inited)) {
        return -OAL_EBUSY;
    }

    region_num = pst_pcie_res->regions.region_nums;
    region_base = pst_pcie_res->regions.pst_regions;

    for (index = 0; index < region_num; index++, region_base++) {
        if (region_base->vaddr == NULL) {
            continue;
        }
        if (region_base->vaddr != NULL) {
            uint32_t size = region_base->size > 256 ? 256 : region_base->size; /* 表示最多打印256字节 */
            oal_io_print("dump region[%d],name:%s, cpu addr:0x%llx\n",
                         index, region_base->name, region_base->cpu_start);
            oal_print_hex_dump((uint8_t *)region_base->vaddr, size, HEX_DUMP_GROUP_SIZE, "pci mem  ");
        }
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_dump_all_regions(struct kobject *dev, struct kobj_attribute *attr,
                                             const char *buf, oal_pcie_res *pst_pcie_res)
{
    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pci not init!\n");
        return -OAL_EBUSY;
    }

    oal_pcie_regions_info_dump(pst_pcie_res);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_debug_read32(struct kobject *dev, struct kobj_attribute *attr,
                                         const char *buf, oal_pcie_res *pst_pcie_res)
{
    /* echo read32 address > debug */
    int32_t ret;
    pci_addr_map addr_map;
    uint32_t cpu_address = 0xFFFFFFFF;
    uint32_t value;
    oal_pci_dev_stru *pst_pci_dev = NULL;

    if ((sscanf_s(buf, "0x%x", &cpu_address) != 1)) {
        pci_print_log(PCI_LOG_ERR, "read32 argument invalid,[%s]", buf);
        return -OAL_EINVAL;
    }

    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pcie_res, cpu_address, &addr_map);
    if (ret != OAL_SUCC) {
        oal_io_print("pcie_read32 0x%8x unmap, read failed!\n", cpu_address);
        return -OAL_EBUSY;
    }
    value = oal_readl((void *)addr_map.va);
    oal_pcie_print_bits(&value, sizeof(uint32_t));
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_debug_write32(struct kobject *dev, struct kobj_attribute *attr,
                                          const char *buf, oal_pcie_res *pst_pcie_res)
{
    /* echo write32 address value > debug */
    int32_t ret;
    pci_addr_map addr_map;
    uint32_t cpu_address = 0xFFFFFFFF;
    uint32_t value, old;
    oal_pci_dev_stru *pst_pci_dev = NULL;
    if ((sscanf_s(buf, "0x%x 0x%x", &cpu_address, &value) != 2)) {
        return -OAL_EINVAL;
    }

    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pcie_res, cpu_address, &addr_map);
    if (ret != OAL_SUCC) {
        oal_io_print("pcie_write32 0x%8x unmap, read failed!\n", cpu_address);
        return -OAL_ENODEV;
    }
    old = oal_readl((void *)addr_map.va);
    oal_writel(value, (void *)addr_map.va);
    oal_io_print("pcie_write32  change 0x%8x from 0x%8x to 0x%8x callback-read= 0x%8x\n",
                 cpu_address, old, value, oal_readl((void *)addr_map.va));
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_debug_read_dsm32(struct kobject *dev, struct kobj_attribute *attr,
                                             const char *buf, oal_pcie_res *pst_pcie_res)
{
    int32_t ret;
    uint32_t val, type;
    if ((sscanf_s(buf, "%u", &type) != 1)) {
        pci_print_log(PCI_LOG_ERR, "host read_dsm32 argument invalid,[%s]", buf);
        return -OAL_EINVAL;
    }

    ret = oal_pcie_read_dsm32(pst_pcie_res, type, &val);
    if (ret == OAL_SUCC) {
        pci_print_log(PCI_LOG_INFO, "read_dsm32 type=%u, val=%u", type, val);
    }

    return ret;
}

OAL_STATIC int32_t oal_pcie_debug_write_dsm32(struct kobject *dev, struct kobj_attribute *attr,
                                              const char *buf, oal_pcie_res *pst_pcie_res)
{
    int32_t ret;
    uint32_t val, type;
    if ((sscanf_s(buf, "%u %u", &type, &val) != 2)) {
        pci_print_log(PCI_LOG_ERR, "host write_dsm32 argument invalid,[%s]", buf);
        return -OAL_EINVAL;
    }

    ret = oal_pcie_write_dsm32(pst_pcie_res, type, val);
    if (ret == OAL_SUCC) {
        pci_print_log(PCI_LOG_INFO, "write_dsm32 type=%u, val=%u", type, val);
    }

    return ret;
}

OAL_STATIC int32_t oal_pcie_debug_host_read32(struct kobject *dev, struct kobj_attribute *attr,
                                              const char *buf, oal_pcie_res *pst_pcie_res)
{
    /* echo host_read32 address > debug */
    unsigned long cpu_address, align_addr;
    void *pst_vaddr = NULL;
    int32_t value;

    if ((sscanf_s(buf, "0x%lx", &cpu_address) != 1)) {
        pci_print_log(PCI_LOG_ERR, "host read32 argument invalid,[%s]", buf);
        return -OAL_EINVAL;
    }

    align_addr = OAL_ROUND_DOWN(cpu_address, PAGE_SIZE);

    pst_vaddr = oal_ioremap_nocache(align_addr, PAGE_SIZE);
    if (pst_vaddr == OAL_SUCC) {
        oal_io_print("pcie_host_read32 0x%lx map failed!\n", align_addr);
        return -OAL_EBUSY;
    }

    value = oal_readl(pst_vaddr + (cpu_address - align_addr));
    oal_pcie_print_bits(&value, sizeof(uint32_t));

    oal_iounmap(pst_vaddr);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_debug_host_write32(struct kobject *dev, struct kobj_attribute *attr,
                                               const char *buf, oal_pcie_res *pst_pcie_res)
{
    /* echo host_write32 address value > debug */
    unsigned long cpu_address, align_addr;
    void *pst_vaddr = NULL;
    void *pst_align_addr = NULL;
    uint32_t old, value;

    if ((sscanf_s(buf, "0x%lx 0x%x", &cpu_address, &value) != 2)) {
        return -OAL_EINVAL;
    }

    align_addr = OAL_ROUND_DOWN(cpu_address, PAGE_SIZE);

    pst_vaddr = oal_ioremap_nocache(align_addr, PAGE_SIZE);
    if (pst_vaddr == OAL_SUCC) {
        oal_io_print("pcie_host_write32 0x%lx map failed!\n", align_addr);
        return -OAL_EBUSY;
    }

    pst_align_addr = pst_vaddr + (cpu_address - align_addr);

    old = oal_readl((void *)pst_align_addr);
    oal_writel(value, (void *)pst_align_addr);
    oal_io_print("host_write32  change 0x%lx from 0x%8x to 0x%8x callback-read= 0x%8x\n",
                 cpu_address, old, value, oal_readl((void *)pst_align_addr));
    oal_iounmap(pst_vaddr);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_debug_read16(struct kobject *dev, struct kobj_attribute *attr,
                                         const char *buf, oal_pcie_res *pst_pcie_res)
{
    /* echo read32 address > debug */
    int32_t ret;
    pci_addr_map addr_map;
    uint32_t cpu_address = 0xFFFFFFFF;
    uint32_t value;
    oal_pci_dev_stru *pst_pci_dev = NULL;

    if ((sscanf_s(buf, "0x%x", &cpu_address) != 1)) {
        return -OAL_EINVAL;
    }

    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pcie_res, cpu_address, &addr_map);
    if (ret != OAL_SUCC) {
        oal_io_print("pcie_read16 0x%8x unmap, read failed!\n", cpu_address);
        return -OAL_ENODEV;
    }
    value = readw((void *)addr_map.va);
    oal_pcie_print_bits(&value, sizeof(uint16_t));
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_debug_write16(struct kobject *dev, struct kobj_attribute *attr,
                                          const char *buf, oal_pcie_res *pst_pcie_res)
{
    /* echo write32 address value > debug */
    int32_t ret;
    pci_addr_map addr_map;
    uint32_t cpu_address = 0xFFFFFFFF;
    uint32_t value, old;
    oal_pci_dev_stru *pst_pci_dev = NULL;
    if ((sscanf_s(buf, "0x%x 0x%x", &cpu_address, &value) != 2)) {
        return -OAL_EINVAL;
    }

    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pcie_res, cpu_address, &addr_map);
    if (ret != OAL_SUCC) {
        oal_io_print("pcie_write16 0x%8x unmap, read failed!\n", cpu_address);
        return -OAL_ENODEV;
    }
    old = readw((void *)addr_map.va);
    writew((uint16_t)value, (void *)addr_map.va);
    oal_io_print("pcie_write16  change 0x%8x from 0x%4x to 0x%4x callback-read= 0x%4x\n",
                 cpu_address, old, (uint16_t)value, readw((void *)addr_map.va));
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_saveconfigmem(oal_pcie_res *pst_pcie_res, const char *file_name,
                                          uint32_t cpu_address, uint32_t length)
{
    struct file *fp = NULL;
    uint32_t index;
    int32_t ret = OAL_SUCC;
    mm_segment_t fs;
    void *pst_buf = NULL;

    oal_pci_dev_stru *pst_pci_dev = NULL;

    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    if (length == 0) { /* 0 is invalid */
        pci_print_log(PCI_LOG_ERR, "input len is 0");
        return -OAL_EINVAL;
    }

    length = padding_m(length, 4); /* 计算4字节对齐后的长度，默认进位 */

    pst_buf = vmalloc(length);
    if (pst_buf == NULL) {
        pci_print_log(PCI_LOG_ERR, "savemem pst_buf is null, vmalloc size %u failed!", length);
        return -OAL_ENOMEM;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(file_name, O_RDWR | O_CREAT, 0664);
    if (oal_is_err_or_null(fp)) {
        pci_print_log(PCI_LOG_ERR, "create file error,fp = 0x%p, filename is [%s]\n", fp, file_name);
        vfree(pst_buf);
        set_fs(fs);
        return -OAL_EINVAL;
    }

    pci_print_log(PCI_LOG_DBG, "savemem cpu:0x%8x len:%u save_path:%s", cpu_address, length, file_name);

    oal_reference(ret);

    for (index = 0; index < length; index += sizeof(uint32_t)) { /* 每次偏移4字节 */
        uint32_t reg = 0;
        ret = oal_pci_read_config_dword(pst_pci_dev, cpu_address + index, &reg);
        if (ret) {
            pci_print_log(PCI_LOG_ERR, "read 0x%x failed, ret=%d", cpu_address + index, ret);
            break;
        }

        oal_writel(reg, pst_buf + index);
    }

    if (index) {
        pci_print_log(PCI_LOG_INFO, "savemem cpu:0x%8x len:%u save_path:%s done", cpu_address, index, file_name);
        ret = oal_kernel_file_write(fp, pst_buf, length);
        if (ret < 0) {
            pci_print_log(PCI_LOG_ERR, "vfs write failed!");
        }
    } else {
        pci_print_log(PCI_LOG_WARN, "savemem cpu:0x%8x len:%u save_path:%s failed!",
                      cpu_address, length, file_name);
        ret = -OAL_EINVAL;
    }

#if (KERNEL_VERSION(2, 6, 35) <= LINUX_VERSION_CODE)
    vfs_fsync(fp, 0);
#else
    vfs_fsync(fp, fp->f_path.dentry, 0);
#endif

    filp_close(fp, NULL);
    set_fs(fs);
    vfree(pst_buf);
    return ret;
}

OAL_STATIC int32_t oal_pcie_savemem(oal_pcie_res *pst_pcie_res, const char *file_name,
                                    uint32_t cpu_address, uint32_t length)
{
    struct file *fp = NULL;
    uint32_t index, value;
    int32_t ret = OAL_SUCC;
    pci_addr_map addr_map;
    mm_segment_t fs;
    void *pst_buf = NULL;

    oal_pci_dev_stru *pst_pci_dev = NULL;

    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    if (length == 0) { /* 0 is invalid */
        pci_print_log(PCI_LOG_ERR, "input len is 0");
        return -OAL_EINVAL;
    }

    length = padding_m(length, 4); /* 计算4字节对齐后的长度，默认进位 */

    pst_buf = vmalloc(length);
    if (pst_buf == NULL) {
        pci_print_log(PCI_LOG_ERR, "savemem pst_buf is null, vmalloc size %u failed!", length);
        return -OAL_ENOMEM;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(file_name, O_RDWR | O_CREAT, 0664);
    if (oal_is_err_or_null(fp)) {
        pci_print_log(PCI_LOG_ERR, "create file error,fp = 0x%p, filename is [%s]\n", fp, file_name);
        vfree(pst_buf);
        set_fs(fs);
        return -OAL_EINVAL;
    }

    for (index = 0; index < length; index += sizeof(uint32_t)) { /* 每次偏移4字节 */
        ret = oal_pcie_inbound_ca_to_va(pst_pcie_res, cpu_address + index, &addr_map);
        if (ret != OAL_SUCC) {
            pci_print_log(PCI_LOG_ERR, "savemem address 0x%8x invalid", cpu_address + index);
            break;
        }
        /* 这里有可能保存的是寄存器区域，按4字节对齐访问 */
        value = oal_readl((void *)addr_map.va);
        oal_writel(value, pst_buf + index);
    }

    if (index) {
        pci_print_log(PCI_LOG_INFO, "savemem cpu:0x%8x len:%u save_path:%s done", cpu_address, index, file_name);
        ret = oal_kernel_file_write(fp, pst_buf, length);
        if (ret < 0) {
            pci_print_log(PCI_LOG_ERR, "vfs write failed!");
        }
    } else {
        pci_print_log(PCI_LOG_WARN, "savemem cpu:0x%8x len:%u save_path:%s failed!",
                      cpu_address, length, file_name);
        ret = -OAL_EINVAL;
    }

#if (KERNEL_VERSION(2, 6, 35) <= LINUX_VERSION_CODE)
    vfs_fsync(fp, 0);
#else
    vfs_fsync(fp, fp->f_path.dentry, 0);
#endif

    filp_close(fp, NULL);
    set_fs(fs);
    vfree(pst_buf);
    return ret;
}

OAL_STATIC int32_t oal_pcie_save_hostmem(oal_pcie_res *pst_pcie_res, const char *file_name,
                                         uintptr_t host_address, uint32_t length)
{
    struct file *fp = NULL;
    uint32_t index, value;
    int32_t ret = OAL_SUCC;
    mm_segment_t fs;
    void *pst_buf = NULL;
    void *vaddr = NULL;

    oal_pci_dev_stru *pst_pci_dev = NULL;

    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    if (length == 0) { /* 0 is invalid */
        pci_print_log(PCI_LOG_ERR, "input len is 0");
        return -OAL_EINVAL;
    }

    length = padding_m(length, 4); /* 计算4字节对齐后的长度，默认进位 */

    vaddr = oal_ioremap_nocache(host_address, length);
    if (vaddr == NULL) {
        pci_print_log(PCI_LOG_ERR, "ioremap %lx , len:%u ,fail", host_address, length);
        return -OAL_ENOMEM;
    }

    pst_buf = vmalloc(length);
    if (pst_buf == NULL) {
        pci_print_log(PCI_LOG_ERR, "savemem pst_buf is null, vmalloc size %u failed!", length);
        return -OAL_ENOMEM;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(file_name, O_RDWR | O_CREAT, 0664);
    if (oal_is_err_or_null(fp)) {
        pci_print_log(PCI_LOG_ERR, "create file error,fp = 0x%p, filename is [%s]\n", fp, file_name);
        set_fs(fs);
        vfree(pst_buf);
        oal_iounmap(vaddr);
        return -OAL_EINVAL;
    }

    for (index = 0; index < length; index += sizeof(uint32_t)) {
        /* 这里有可能保存的是寄存器区域，按4字节对齐访问 */
        value = oal_readl(vaddr + index);
        oal_writel(value, pst_buf + index);
    }

    if (index) {
        pci_print_log(PCI_LOG_INFO, "save_hostmem cpu:0x%lx len:%u save_path:%s done",
                      host_address, index, file_name);
        ret = oal_kernel_file_write(fp, pst_buf, length);
        if (ret < 0) {
            pci_print_log(PCI_LOG_ERR, "vfs write failed!");
        }
    } else {
        pci_print_log(PCI_LOG_WARN, "save_hostmem cpu:0x%lx len:%u save_path:%s failed!",
                      host_address, length, file_name);
        ret = -OAL_EINVAL;
    }

#if (KERNEL_VERSION(2, 6, 35) <= LINUX_VERSION_CODE)
    vfs_fsync(fp, 0);
#else
    vfs_fsync(fp, fp->f_path.dentry, 0);
#endif

    filp_close(fp, NULL);
    set_fs(fs);
    vfree(pst_buf);
    oal_iounmap(vaddr);
    return ret;
}

OAL_STATIC int32_t oal_pcie_debug_savemem(struct kobject *dev, struct kobj_attribute *attr,
                                          const char *buf, oal_pcie_res *pst_pcie_res)
{
    char file_name[PCIE_DEBUG_MSG_LEN];
    uint32_t cpu_address, length;
    int32_t ret;

    if (strlen(buf) >= sizeof(file_name)) {
        oal_io_print("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    if ((sscanf_s(buf, "0x%x %u %s", &cpu_address, &length, file_name, sizeof(file_name)) != 3)) {
        return -OAL_EINVAL;
    }

    ret = oal_pcie_savemem(pst_pcie_res, file_name, cpu_address, length);
    return ret;
}

OAL_STATIC int32_t oal_pcie_debug_saveconfigmem(struct kobject *dev, struct kobj_attribute *attr,
                                                const char *buf, oal_pcie_res *pst_pcie_res)
{
    char file_name[PCIE_DEBUG_MSG_LEN];
    uint32_t cpu_address, length;
    int32_t ret;

    if (strlen(buf) >= sizeof(file_name)) {
        oal_io_print("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    if ((sscanf_s(buf, "0x%x %u %s", &cpu_address, &length, file_name, sizeof(file_name)) != 3)) {
        return -OAL_EINVAL;
    }

    ret = oal_pcie_saveconfigmem(pst_pcie_res, file_name, cpu_address, length);
    return ret;
}

OAL_STATIC int32_t oal_pcie_debug_save_hostmem(struct kobject *dev, struct kobj_attribute *attr,
                                               const char *buf, oal_pcie_res *pst_pcie_res)
{
    char file_name[PCIE_DEBUG_MSG_LEN];
    uintptr_t host_address;
    uint32_t length;
    int32_t ret;

    if (strlen(buf) >= sizeof(file_name)) {
        oal_io_print("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    if ((sscanf_s(buf, "0x%lx %u %s", &host_address, &length, file_name, sizeof(file_name)) != 3)) {
        return -OAL_EINVAL;
    }

    ret = oal_pcie_save_hostmem(pst_pcie_res, file_name, host_address, length);
    return ret;
}

int32_t oal_pcie_print_device_mem(oal_pcie_res *pst_pcie_res, uint32_t cpu_address, uint32_t length)
{
    /* echo readmem address length(hex) > debug */
    uint32_t index;
    int32_t ret;
    pci_addr_map addr_map;
    void *print_buf = NULL;
    oal_pci_dev_stru *pst_pci_dev = NULL;

    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    if (length == 0) { /* 0 is invalid */
        pci_print_log(PCI_LOG_ERR, "input len is 0");
        return -OAL_EINVAL;
    }

    length = padding_m(length, 4); /* 计算4字节对齐后的长度，默认进位 */
    print_buf = vmalloc(length);
    if (print_buf == NULL) {
        return -OAL_EINVAL;
    }

    for (index = 0; index < length; index += sizeof(uint32_t)) { /* 每次偏移4字节 */
        ret = oal_pcie_inbound_ca_to_va(pst_pcie_res, cpu_address + index, &addr_map);
        if (ret != OAL_SUCC) {
            pci_print_log(PCI_LOG_ERR, "readmem address 0x%8x invalid", cpu_address + index);
            break;
        }
        *(uint32_t *)(print_buf + index) = oal_readl((void *)addr_map.va);
    }

    if (index) {
        pci_print_log(PCI_LOG_INFO, "readmem cpu:0x%8x len:%u, va:0x%lx, pa:0x%lx  done",
                      cpu_address, index, addr_map.va, addr_map.pa);
        print_hex_dump(KERN_INFO, "readmem: ", DUMP_PREFIX_OFFSET, 16, 4,
                       print_buf, length, true); /* 内核函数固定的传参 */
    } else {
        pci_print_log(PCI_LOG_WARN, "readmem cpu:0x%8x len:%u  failed!", cpu_address, length);
        vfree(print_buf);
        return -OAL_EINVAL;
    }

    vfree(print_buf);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_debug_readmem(struct kobject *dev, struct kobj_attribute *attr,
                                          const char *buf, oal_pcie_res *pst_pcie_res)
{
    /* echo readmem address length(hex) > debug */
    uint32_t cpu_address, length;

    if ((sscanf_s(buf, "0x%x %u", &cpu_address, &length) != 2)) {
        return -OAL_EINVAL;
    }

    (void)oal_pcie_print_device_mem(pst_pcie_res, cpu_address, length);

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_debug_readmem_config(struct kobject *dev, struct kobj_attribute *attr,
                                                 const char *buf, oal_pcie_res *pst_pcie_res)
{
    /* echo readmem address length(hex) > debug */
    uint32_t cpu_address, length, index;
    int32_t ret;
    uint32_t reg = 0;
    void *print_buf = NULL;
    oal_pci_dev_stru *pst_pci_dev = NULL;

    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    if ((sscanf_s(buf, "0x%x %u", &cpu_address, &length) != 2)) {
        return -OAL_EINVAL;
    }

    if (length == 0) { /* 0 is invalid */
        pci_print_log(PCI_LOG_ERR, "input len is 0");
        return -OAL_EINVAL;
    }

    length = padding_m(length, 4); /* 计算4字节对齐后的长度，默认进位 */
    print_buf = vmalloc(length);
    if (print_buf == NULL) {
        return -OAL_EINVAL;
    }

    for (index = 0; index < length; index += sizeof(uint32_t)) { /* 每次偏移4字节 */
        ret = oal_pci_read_config_dword(pst_pci_dev, cpu_address + index, &reg);
        if (ret) {
            pci_print_log(PCI_LOG_ERR, "read 0x%x failed, ret=%d", cpu_address + index, ret);
            break;
        }
        *(uint32_t *)(print_buf + index) = reg;
    }

    if (index) {
        pci_print_log(PCI_LOG_INFO, "readmem cpu:0x%8x len:%u  done", cpu_address, index);
        oal_print_hex_dump(print_buf, length, HEX_DUMP_GROUP_SIZE, "readmem: ");
    } else {
        pci_print_log(PCI_LOG_WARN, "readmem cpu:0x%8x len:%u  failed!", cpu_address, length);
        vfree(print_buf);
        return -OAL_EINVAL;
    }

    vfree(print_buf);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_send_test_pkt(int32_t num)
{
    uint32_t cmd_len = 1500;
    oal_netbuf_stru *pst_netbuf = NULL;
    struct hcc_transfer_param st_hcc_transfer_param = {0};
    struct hcc_handler *hcc = hcc_get_110x_handler();
    if (hcc == NULL) {
        return -OAL_EFAIL;
    }

    if (num == 0) {
        return -OAL_EINVAL;
    }

    do {
        pst_netbuf = hcc_netbuf_alloc(cmd_len);
        if (pst_netbuf == NULL) {
            oal_io_print("hwifi alloc skb fail.\n");
            return -OAL_EFAIL;
        }

        memset_s(oal_netbuf_put(pst_netbuf, cmd_len), cmd_len, 0x5a, cmd_len);

        hcc_hdr_param_init(&st_hcc_transfer_param,
                           HCC_ACTION_TYPE_TEST,
                           HCC_TEST_SUBTYPE_CMD,
                           0,
                           HCC_FC_WAIT,
                           0);
        if (OAL_SUCC != hcc_tx(hcc, pst_netbuf, &st_hcc_transfer_param)) {
            oal_netbuf_free(pst_netbuf);
            oal_io_print("hcc tx failed\n");
            return -OAL_EFAIL;
        }
    } while (--num);

    return OAL_SUCC;
}

OAL_STATIC void oal_pcie_outbound_test_init(oal_pcie_res *pst_pcie_res, oal_pci_dev_stru *pst_pci_dev,
                                            uint32_t outbound_size, dma_addr_t outbound_dma_addr)
{
    if (pst_pcie_res->chip_info.edma_support == OAL_TRUE) {
        void *inbound_addr = pst_pcie_res->st_iatu_bar.st_region.vaddr;
        if (pst_pcie_res->revision == PCIE_REVISION_4_70A) {
            oal_pci_write_config_dword(pst_pci_dev, 0x700 + 0x200, 0x1);        /* view index */
            oal_pci_write_config_dword(pst_pci_dev, 0x700 + 0x204, 0x0);        /* ctrl 1 */
            oal_pci_write_config_dword(pst_pci_dev, 0x700 + 0x208, 0x80000000); /* ctrl 2 */
            oal_pci_write_config_dword(pst_pci_dev, 0x700 + 0x20c, 0x80000000); /* base lower */
            oal_pci_write_config_dword(pst_pci_dev, 0x700 + 0x210, 0);          /* base upper */
            oal_pci_write_config_dword(pst_pci_dev, 0x700 + 0x214,
                                       0x70000000 + outbound_size - 1); /* limit */
            oal_pci_write_config_dword(pst_pci_dev, 0x700 + 0x218,
                                       (uint32_t)outbound_dma_addr); /* target lower */
            oal_pci_write_config_dword(pst_pci_dev, 0x700 + 0x21c, 0); /* target upper */
        } else if (pst_pcie_res->chip_info.membar_support == OAL_TRUE) {
            iatu_region_ctrl_2_off ctr2;
            uint32_t index = 0;
            if (inbound_addr == NULL) {
                pci_print_log(PCI_LOG_ERR, "inbound_addr is null");
                return;
            }
            pci_print_log(PCI_LOG_INFO, "outbound config");
            oal_writel(0x0,
                       inbound_addr +
                       hi_pci_iatu_region_ctrl_1_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
            ctr2.as_dword = 0;
            ctr2.bits.region_en = 1;
            ctr2.bits.bar_num = 0x0;
            oal_writel(ctr2.as_dword,
                       inbound_addr +
                       hi_pci_iatu_region_ctrl_2_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));

            oal_writel(0x80000000,
                       inbound_addr +
                       hi_pci_iatu_lwr_base_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
            oal_writel(0x0,
                       inbound_addr +
                       hi_pci_iatu_upper_base_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
            oal_writel(0x80000000 + outbound_size - 1,
                       inbound_addr +
                       hi_pci_iatu_limit_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));

            oal_writel((uint32_t)outbound_dma_addr,
                       inbound_addr +
                       hi_pci_iatu_lwr_target_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
            oal_writel((uint32_t)(outbound_dma_addr >> 32),
                       inbound_addr +
                       hi_pci_iatu_upper_target_addr_off_outbound_i(hi_pci_iatu_outbound_base_off(index)));
        } else {
            pci_print_log(PCI_LOG_ERR, "unkown pcie ip revision :0x%x", pst_pcie_res->revision);
        }
    }

    if (pst_pcie_res->chip_info.ete_support == OAL_TRUE) {
        int32_t ret = OAL_SUCC;
        uint64_t host_iova = 0;
        uint64_t devva = 0;
        ret = pcie_if_hostca_to_devva(0, (uint64_t)outbound_dma_addr, &devva);
        if (ret == OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_if_hostca_to_devva iova=0x%llx -> devva=0x%llx",
                                 (uint64_t)outbound_dma_addr, devva);
            ret = pcie_if_devva_to_hostca(0, devva, &host_iova);
            if (ret == OAL_SUCC) {
                oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie_if_devva_to_hostca devva=0x%llx -> iova=0x%llx",
                                     devva, host_iova);
            } else {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "pcie_if_devva_to_hostca failed devva=0x%llx", devva);
            }
        }
    }
}

/*
 * 测试outbound是否生效，返回DDR地址，
 * 通过SSI或者WCPU 读写Device 侧PCIe Slave地址 查看DDR是否有改变,
 * 1103 Slave 空间为256M
 */
OAL_STATIC int32_t oal_pcie_outbound_test(oal_pcie_res *pst_pcie_res, const char *buf)
{
    OAL_STATIC dma_addr_t outbound_dma_addr = 0;
    uint32_t outbound_size = 4096;
    OAL_STATIC oal_uint *outbound_vaddr = NULL;
    char casename[PCIE_DEBUG_MSG_LEN] = {0};
    oal_pci_dev_stru *pst_pci_dev;

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    if (strlen(buf) >= sizeof(casename)) {
        oal_io_print("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    /* Just for debug */
    if ((sscanf_s(buf, "%s", casename, sizeof(casename)) != 1)) {
        pci_print_log(PCI_LOG_ERR, "invalid agrument");
        return -OAL_EINVAL;
    } else {
        /* dump */
        if (outbound_vaddr) {
            oal_print_hex_dump((uint8_t *)outbound_vaddr, outbound_size, HEX_DUMP_GROUP_SIZE, "outbound: ");
        }
    }
    pci_print_log(PCI_LOG_INFO, "outbound:[%s]", casename);
    if (!oal_strcmp(casename, "rebound")) {
        if (outbound_vaddr) {
            dma_free_coherent(&pst_pci_dev->dev, outbound_size, outbound_vaddr, outbound_dma_addr);
        }
        outbound_vaddr = dma_alloc_coherent(&pst_pci_dev->dev, outbound_size,
                                            &outbound_dma_addr,
                                            GFP_KERNEL);
        if (outbound_vaddr) {
            memset_s(outbound_vaddr, outbound_size, 0x38, outbound_size);
            /* set outbound */
            oal_io_print("host dma addr:0x%lx , vaddr: 0x%p\n", (uintptr_t)outbound_dma_addr, outbound_vaddr);
            oal_pcie_outbound_test_init(pst_pcie_res, pst_pci_dev, outbound_size, outbound_dma_addr);
        }
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_addr_switch_test(oal_pcie_res *pst_pcie_res, const char *buf)
{
    int32_t ret = OAL_SUCC;
    if (pst_pcie_res->chip_info.addr_info.glb_ctrl != 0) {
        uint64_t host_va = 0;
        uint64_t dev_cpuaddr = 0;
        ret = oal_pcie_devca_to_hostva(0, pst_pcie_res->chip_info.addr_info.glb_ctrl, &host_va);
        if (ret == OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "oal_pcie_devca_to_hostva 0x%x -> 0x%llx, value=0x%x",
                                 pst_pcie_res->chip_info.addr_info.glb_ctrl, host_va,
                                 oal_readl((void*)(uintptr_t)host_va));
            /* master hostva->devca */
            ret = oal_pcie_get_dev_ca(0, (void*)(uintptr_t)host_va, &dev_cpuaddr);
            if (ret == OAL_SUCC) {
                oal_print_hi11xx_log(HI11XX_LOG_INFO, "oal_pcie_get_dev_ca 0x%llx -> 0x%x",
                                     host_va, (uint32_t)dev_cpuaddr);
            } else {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "oal_pcie_get_dev_ca 0x%llx  convert failed",
                                     host_va);
            }
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "oal_pcie_devca_to_hostva 0x%x  convert failed",
                                 pst_pcie_res->chip_info.addr_info.glb_ctrl);
        }
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "glb_ctrl is 0");
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_device_memcheck(oal_pcie_res *pst_pcie_res, uint32_t cpuaddr,
                                            uint32_t size, uint32_t data)
{
    volatile unsigned int data_rd;
    volatile unsigned int data_wt;
    unsigned int i = 0;
    unsigned int mode = 0;
    int32_t ret;
    pci_addr_map addr_map;
    oal_pci_dev_stru *pst_pci_dev;

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    if (cpuaddr & (0x3)) {
        pci_print_log(PCI_LOG_ERR, "invalid cpu address, must align to 4 bytes:%u", cpuaddr);
        return -OAL_EINVAL;
    }

    if (size & 0x3) {
        pci_print_log(PCI_LOG_ERR, "invalid size, must align to 4 bytes:%u", size);
        return -OAL_EINVAL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pcie_res, cpuaddr + size - 1, &addr_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "unmap device address 0x%x, size:%u", cpuaddr, size);
        return -OAL_EINVAL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pcie_res, cpuaddr, &addr_map);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "unmap device address 0x%x", cpuaddr);
        return -OAL_EINVAL;
    }

    for (i = 0, mode = 0; i < (size); i += sizeof(uint32_t), mode++) { /* 每次偏移4字节，直到超过size */
        if (mode % 4 < 2) { /* 两块4字节地址写data，两块4字节地址写~data，测试mem用 */
            data_wt = data;
        } else {
            data_wt = ~data;
        }

        oal_writel(data_wt, (void *)(addr_map.va + i));
        data_rd = oal_readl((void *)(addr_map.va + i));
        if (data_rd != data_wt) {
            pci_print_log(PCI_LOG_ERR, "bad address :0x%8x ,write: 0x%8x ,read: 0x%8x",
                          cpuaddr + i, data_wt, data_rd);
            return -OAL_EFAIL;
        }
    }
    return OAL_SUCC;
}

typedef struct _memcheck_item_ {
    uint32_t address; /* device cpu address */
    uint32_t size;    /* device cpu address */
} memcheck_item;

/* 需要WCPU代码在bootloader 阶段 */
OAL_STATIC int32_t oal_pcie_device_memcheck_auto(oal_pcie_res *pst_pcie_res)
{
    int32_t ret, i;
    memcheck_item test_address[] = {
        { 0x00004000, 544 * 1024 }, /* 544KB */
        { 0x20001c00, 409 * 1024 }, /* 409KB */
        { 0x60000000, 576 * 1024 }  /* 576KB */
    };

    pci_print_log(PCI_LOG_INFO, "memcheck start...");
    for (i = 0; i < sizeof(test_address) / sizeof(test_address[0]); i++) {
        pci_print_log(PCI_LOG_INFO, "Test address: 0x%8x ,size: 0x%8x start.",
                      test_address[i].address, test_address[i].size);
        ret = oal_pcie_device_memcheck(pst_pcie_res, test_address[i].address, test_address[i].size, 0xFFFFFFFF);
        if (ret != OAL_SUCC) {
            break;
        }

        ret = oal_pcie_device_memcheck(pst_pcie_res, test_address[i].address, test_address[i].size, 0x00000000);
        if (ret != OAL_SUCC) {
            break;
        }

        ret = oal_pcie_device_memcheck(pst_pcie_res, test_address[i].address, test_address[i].size, 0x5a5a5a5a);
        if (ret != OAL_SUCC) {
            break;
        }

        ret = oal_pcie_device_memcheck(pst_pcie_res, test_address[i].address, test_address[i].size, 0xa5a5a5a5);
        if (ret != OAL_SUCC) {
            break;
        }
        pci_print_log(PCI_LOG_INFO, "Test address: 0x%8x ,size: 0x%8x OK.",
                      test_address[i].address, test_address[i].size);
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_debug_wlan_power_on(struct kobject *dev, struct kobj_attribute *attr,
                                                const char *buf, oal_pcie_res *pst_pcie_res)
{
    oal_pci_dev_stru *pst_pci_dev = NULL;

    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }
    wlan_power_on();
    return OAL_SUCC;
}

#ifdef CONFIG_ARCH_KIRIN_PCIE
OAL_STATIC int32_t oal_pcie_resume_handler(void *data)
{
    oal_reference(data);
    /* 这里保证解复位EP控制器时efuse已经稳定 */
    board_host_wakeup_dev_set(GPIO_HIGHLEVEL);
    oal_msleep(25); /* 这里要用GPIO 做ACK 延迟不可靠, MPW2 硬件唤醒15ms,软件6ms */
    pci_print_log(PCI_LOG_DBG, "oal_pcie_resume_handler, pull up gpio");
    return 0;
}
#endif

OAL_STATIC int32_t oal_pcie_testcase_transfer_res_init(oal_pcie_res *pst_pcie_res,
                                                       oal_pci_dev_stru *pst_pci_dev,
                                                       const char *buf)
{
    int32_t ret;

    ret = oal_pcie_transfer_res_init(pst_pcie_res);
    if (ret == OAL_SUCC) {
        pci_print_log(PCI_LOG_INFO, "oal_pcie_transfer_res_init:SUCC");
    } else {
        pci_print_log(PCI_LOG_ERR, "oal_pcie_transfer_res_init:FAIL");
    }
    return ret;
}

OAL_STATIC int32_t oal_pcie_testcase_send_test_pkt(oal_pcie_res *pst_pcie_res,
                                                   oal_pci_dev_stru *pst_pci_dev,
                                                   const char *buf)
{
    oal_pcie_send_test_pkt(1); /* 表示测试循环次数为1 */
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_send_test_pkt2(oal_pcie_res *pst_pcie_res,
                                                    oal_pci_dev_stru *pst_pci_dev,
                                                    const char *buf)
{
    oal_pcie_send_test_pkt(2); /* 表示测试循环次数为2 */
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_send_test_pkt3(oal_pcie_res *pst_pcie_res,
                                                    oal_pci_dev_stru *pst_pci_dev,
                                                    const char *buf)
{
    oal_pcie_send_test_pkt(3); /* 表示测试循环次数为3 */
    return OAL_SUCC;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE_PERFORMANCE
OAL_STATIC int32_t oal_pcie_testcase_pcie_mips_show(oal_pcie_res *pst_pcie_res,
                                                    oal_pci_dev_stru *pst_pci_dev,
                                                    const char *buf)
{
    oal_pcie_mips_show();
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_pcie_mips_clear(oal_pcie_res *pst_pcie_res,
                                                     oal_pci_dev_stru *pst_pci_dev,
                                                     const char *buf)
{
    oal_pcie_mips_clear();
    return OAL_SUCC;
}
#endif

OAL_STATIC int32_t oal_pcie_testcase_outbound_test(oal_pcie_res *pst_pcie_res,
                                                   oal_pci_dev_stru *pst_pci_dev,
                                                   const char *buf)
{
    buf = buf + OAL_STRLEN("outbound_test");
    for (; *buf == ' '; buf++);
    oal_pcie_outbound_test(pst_pcie_res, buf);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_addr_switch_test(oal_pcie_res *pst_pcie_res,
                                                      oal_pci_dev_stru *pst_pci_dev,
                                                      const char *buf)
{
    buf = buf + OAL_STRLEN("addr_switch_test");
    for (; *buf == ' '; buf++);
    oal_pcie_addr_switch_test(pst_pcie_res, buf);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_ram_memcheck(oal_pcie_res *pst_pcie_res,
                                                  oal_pci_dev_stru *pst_pci_dev,
                                                  const char *buf)
{
    /* 遍历itcm,dtcm,pktmem,扫内存 */
    oal_pcie_device_memcheck_auto(pst_pcie_res);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_pme_enable(oal_pcie_res *pst_pcie_res,
                                                oal_pci_dev_stru *pst_pci_dev,
                                                const char *buf)
{
    int32_t  ret;
    uint32_t pm;
    uint32_t reg = 0;
    /* Enable PME */
    pm = pci_find_capability(pst_pci_dev, PCI_CAP_ID_PM);
    if (!pm) {
        pci_print_log(PCI_LOG_ERR, "can't get PCI_CAP_ID_PM");
        return OAL_SUCC;
    }
    pci_print_log(PCI_LOG_INFO, "PME OFF : %u", pm);
    ret = oal_pci_read_config_dword(pst_pci_dev, pm + PCI_PM_CTRL, &reg);
    if (ret) {
        pci_print_log(PCI_LOG_ERR, "read %u failed", pm + PCI_PM_CTRL);
        return OAL_SUCC;
    }

    oal_io_print("read %u value:\n", pm + PCI_PM_CTRL);
    oal_pcie_print_bits(&reg, sizeof(reg));

    reg |= 0x100;

    ret = oal_pci_write_config_dword(pst_pci_dev, pm + PCI_PM_CTRL, reg);
    if (ret) {
        pci_print_log(PCI_LOG_ERR, "write %u failed", pm + PCI_PM_CTRL);
    } else {
        pci_print_log(PCI_LOG_INFO, "write %u ok", pm + PCI_PM_CTRL);
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_ringbuf_stat(oal_pcie_res *pst_pcie_res,
                                                  oal_pci_dev_stru *pst_pci_dev,
                                                  const char *buf)
{
    int32_t i;
    uint32_t freecount, usedcount;
    oal_io_print("h2d ringbuf info:\n");
    for (i = 0; i < PCIE_H2D_QTYPE_BUTT; i++) {
        freecount = oal_pcie_h2d_ringbuf_freecount(pst_pcie_res, (pcie_h2d_ringbuf_qtype)i, OAL_TRUE);
        usedcount = pcie_ringbuf_len(&pst_pcie_res->st_ringbuf.st_h2d_buf[i]);
        oal_io_print("qtype:%d , freecount:%u, used_count:%u \n",
                     i, freecount, usedcount);
    }
    oal_io_print("\nd2h ringbuf info:\n");
    freecount = oal_pcie_d2h_ringbuf_freecount(pst_pcie_res, OAL_TRUE);
    usedcount = pcie_ringbuf_len(&pst_pcie_res->st_ringbuf.st_d2h_buf);
    oal_io_print("freecount:%u, used_count:%u \n",
                 freecount, usedcount);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_clear_trans_info(oal_pcie_res *pst_pcie_res,
                                                      oal_pci_dev_stru *pst_pci_dev,
                                                      const char *buf)
{
    memset_s((void *)pst_pcie_res->st_tx_res[PCIE_H2D_QTYPE_NORMAL].stat.tx_burst_cnt,
             sizeof(pst_pcie_res->st_tx_res[PCIE_H2D_QTYPE_NORMAL].stat.tx_burst_cnt),
             0, sizeof(pst_pcie_res->st_tx_res[PCIE_H2D_QTYPE_NORMAL].stat.tx_burst_cnt));

    memset_s((void *)pst_pcie_res->st_rx_res.stat.rx_burst_cnt,
             sizeof(pst_pcie_res->st_rx_res.stat.rx_burst_cnt),
             0, sizeof(pst_pcie_res->st_rx_res.stat.rx_burst_cnt));
    oal_io_print("clear_trans_info done\n");
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_show_trans_info(oal_pcie_res *pst_pcie_res,
                                                     oal_pci_dev_stru *pst_pci_dev,
                                                     const char *buf)
{
    int32_t i;
    oal_io_print("tx h2d trans info:\n");
    for (i = 0; i <
         sizeof(pst_pcie_res->st_tx_res[PCIE_H2D_QTYPE_NORMAL].stat.tx_burst_cnt) /
         sizeof(pst_pcie_res->st_tx_res[PCIE_H2D_QTYPE_NORMAL].stat.tx_burst_cnt[0]);
         i++) {
        oal_io_print("[%3d] burst cnt:%u\n", i,
                     pst_pcie_res->st_tx_res[PCIE_H2D_QTYPE_NORMAL].stat.tx_burst_cnt[i]);
    }

    oal_io_print("\nrx d2h trans info:\n");
    for (i = 0; i <
         sizeof(pst_pcie_res->st_rx_res.stat.rx_burst_cnt) / sizeof(pst_pcie_res->st_rx_res.stat.rx_burst_cnt[0]);
         i++) {
        oal_io_print("[%3d] burst cnt:%u\n", i,
                     pst_pcie_res->st_rx_res.stat.rx_burst_cnt[i]);
    }

    oal_pcie_print_device_transfer_info(pst_pcie_res);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_send_message(oal_pcie_res *pst_pcie_res,
                                                  oal_pci_dev_stru *pst_pci_dev,
                                                  const char *buf)
{
    int32_t ret;
    uint32_t message;

    buf = buf + OAL_STRLEN("send_message");
    for (; *buf == ' '; buf++);
    if ((sscanf_s(buf, "%u", &message) != 1)) {
        pci_print_log(PCI_LOG_ERR, "invalid agrument");
    } else {
        ret = oal_pcie_send_message_to_dev(pst_pcie_res, message);
        pci_print_log(PCI_LOG_INFO, "send pcie message %u to dev %s",
                      message, (ret == OAL_SUCC) ? "succ" : "failed");
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_sched_rx_task(oal_pcie_res *pst_pcie_res,
                                                   oal_pci_dev_stru *pst_pci_dev,
                                                   const char *buf)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);
    if (pst_pci_lres != NULL && pst_pci_lres->pst_bus != NULL) {
        up(&pst_pci_lres->pst_bus->rx_sema);
    } else {
        pci_print_log(PCI_LOG_INFO, "sched failed!");
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_turnoff_message(oal_pcie_res *pst_pcie_res,
                                                     oal_pci_dev_stru *pst_pci_dev,
                                                     const char *buf)
{
#ifdef CONFIG_ARCH_KIRIN_PCIE
    /* 走到这里说明wakelock已经释放，WIFI已经深睡,通知RC/EP下电，发送TurnOff Message */
    /* 下电之前关闭 PCIE HOST 控制器 */
    pci_print_log(PCI_LOG_INFO, "turnoff_message host");
    kirin_pcie_power_notifiy_register(g_kirin_rc_idx, NULL, NULL, NULL);
    kirin_pcie_pm_control(0, g_kirin_rc_idx);
#endif
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_suspend_test(oal_pcie_res *pst_pcie_res,
                                                  oal_pci_dev_stru *pst_pci_dev,
                                                  const char *buf)
{
#ifdef CONFIG_ARCH_KIRIN_PCIE

    int32_t ret__;
    /* 走到这里说明wakelock已经释放，WIFI已经深睡,通知RC/EP下电，发送TurnOff Message */
    /* 下电之前关闭 PCIE HOST 控制器 */
    pci_print_log(PCI_LOG_INFO, "suspend_test host");

    oal_pcie_change_link_state(pst_pcie_res, PCI_WLAN_LINK_DOWN);

    kirin_pcie_power_notifiy_register(g_kirin_rc_idx, NULL, NULL, NULL);
    ret__ = kirin_pcie_pm_control(0, g_kirin_rc_idx);
    board_host_wakeup_dev_set(0);
    if (ret__) {
        pci_print_log(PCI_LOG_ERR, "suspend test power on failed, ret=%d", ret__);
        declare_dft_trace_key_info("suspend test power on failed", OAL_DFT_TRACE_OTHER);
    }
#endif
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_resume_test(oal_pcie_res *pst_pcie_res,
                                                 oal_pci_dev_stru *pst_pci_dev,
                                                 const char *buf)
{
#ifdef CONFIG_ARCH_KIRIN_PCIE
    int32_t ret__;
    /* 走到这里说明wakelock已经释放，WIFI已经深睡,通知RC/EP下电，发送TurnOff Message */
    /* 下电之前关闭 PCIE HOST 控制器 */
    pci_print_log(PCI_LOG_INFO, "resume_test host");
    kirin_pcie_power_notifiy_register(g_kirin_rc_idx, oal_pcie_resume_handler, NULL, NULL);
    ret__ = kirin_pcie_pm_control(1, g_kirin_rc_idx);
    oal_pcie_change_link_state(pst_pcie_res, PCI_WLAN_LINK_WORK_UP);
    if (ret__) {
        pci_print_log(PCI_LOG_ERR, "resume test power on failed, ret=%d", ret__);
        declare_dft_trace_key_info("resume test power on failed", OAL_DFT_TRACE_OTHER);
    }
#endif
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_rc_send_polling(oal_pcie_res *pst_pcie_res,
                                                     oal_pci_dev_stru *pst_pci_dev,
                                                     const char *buf)
{
#ifdef CONFIG_ARCH_KIRIN_PCIE
    oal_pci_dev_stru *pst_rc_dev;
    pst_rc_dev = pci_upstream_bridge(pst_pci_dev);
    if (pst_rc_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "no upstream dev");
    } else {
        uint16_t val = 0;
        oal_pci_read_config_word(pst_rc_dev, oal_pci_pcie_cap(pst_rc_dev) + PCI_EXP_LNKCTL2, &val);
        pci_print_log(PCI_LOG_INFO, "rc polling read 0x%x , value:0x%x",
                      oal_pci_pcie_cap(pst_rc_dev) + PCI_EXP_LNKCTL2,
                      val);
        val |= (1 << 4);
        oal_pci_write_config_word(pst_rc_dev, oal_pci_pcie_cap(pst_rc_dev) + PCI_EXP_LNKCTL2, val);
    }
#endif
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_testcase_ep_send_polling(oal_pcie_res *pst_pcie_res,
                                                     oal_pci_dev_stru *pst_pci_dev,
                                                     const char *buf)
{
    uint16_t val = 0;
    int32_t ret = oal_pci_read_config_word(pst_pci_dev, oal_pci_pcie_cap(pst_pci_dev) + PCI_EXP_LNKCTL2, &val);
    if (ret == 0) {
        pci_print_log(PCI_LOG_INFO, "ep polling read 0x%x , value:0x%x",
                      oal_pci_pcie_cap(pst_pci_dev) + PCI_EXP_LNKCTL2, val);
        val |= (1 << 4);
        ret = oal_pci_write_config_word(pst_pci_dev, oal_pci_pcie_cap(pst_pci_dev) + PCI_EXP_LNKCTL2, val);
        if (ret) {
            pci_print_log(PCI_LOG_WARN, "ep_send_polling write fail");
        }
    }
    return OAL_SUCC;
}

typedef struct _pcie_debug_testcase_ {
    char *name;
    int32_t (*testcase)(oal_pcie_res *pst_pcie_res, oal_pci_dev_stru *pst_pci_dev, const char *buf);
} pcie_debug_testcase;

OAL_STATIC pcie_debug_testcase g_pci_debug_testcases[] = {
    { "transfer_res_init", oal_pcie_testcase_transfer_res_init },
    { "send_test_pkt",     oal_pcie_testcase_send_test_pkt },
    { "send_test_pkt2",    oal_pcie_testcase_send_test_pkt2 },
    { "send_test_pkt3",    oal_pcie_testcase_send_test_pkt3 },
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE_PERFORMANCE
    { "pcie_mips_show", oal_pcie_testcase_pcie_mips_show },
    { "pcie_mips_clear", oal_pcie_testcase_pcie_mips_clear },
#endif
    { "outbound_test",      oal_pcie_testcase_outbound_test },
    { "addr_switch_test",   oal_pcie_testcase_addr_switch_test },
    { "ram_memcheck",       oal_pcie_testcase_ram_memcheck },
    { "PME_ENABLE",         oal_pcie_testcase_pme_enable },
    { "ringbuf_stat",       oal_pcie_testcase_ringbuf_stat },
    { "clear_trans_info",   oal_pcie_testcase_clear_trans_info },
    { "show_trans_info",    oal_pcie_testcase_show_trans_info },
    { "send_message",       oal_pcie_testcase_send_message },
    { "sched_rx_task",      oal_pcie_testcase_sched_rx_task },
    { "turnoff_message",    oal_pcie_testcase_turnoff_message },
    { "suspend_test",       oal_pcie_testcase_suspend_test },
    { "resume_test",        oal_pcie_testcase_resume_test },
    { "rc_send_polling",    oal_pcie_testcase_rc_send_polling },
    { "ep_send_polling",    oal_pcie_testcase_ep_send_polling },
};

OAL_STATIC int32_t oal_pcie_debug_testcase(struct kobject *dev, struct kobj_attribute *attr,
                                           const char *buf, oal_pcie_res *pst_pcie_res)
{
    /* echo readmem address length(hex) > debug */
    int32_t i;

    char casename[PCIE_DEBUG_MSG_LEN] = {0};
    oal_pci_dev_stru *pst_pci_dev = NULL;

    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    if (strlen(buf) >= sizeof(casename)) {
        oal_io_print("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    /* Just for debug */
    if ((sscanf_s(buf, "%s", casename, sizeof(casename)) != 1)) {
        return -OAL_EINVAL;
    }

    for (i = 0; i < oal_array_size(g_pci_debug_testcases); i++) {
        if (g_pci_debug_testcases[i].name) {
            if (!oal_strcmp(g_pci_debug_testcases[i].name, casename)) {
                break;
            }
        }
    }

    if (i == oal_array_size(g_pci_debug_testcases)) {
        pci_print_log(PCI_LOG_WARN, "testcase: %s not found", casename);
        return -OAL_ENODEV;
    }

    if (g_pci_debug_testcases[i].testcase == NULL) {
        pci_print_log(PCI_LOG_WARN, "testcase: %s no func", casename);
        return -OAL_ENODEV;
    }

    pci_print_log(PCI_LOG_INFO, "testcase:[%s] run", casename);
    g_pci_debug_testcases[i].testcase(pst_pcie_res, pst_pci_dev, buf);

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_performance_read(oal_pcie_res *pst_pcie_res, uint32_t cpu_address,
                                             uint32_t length, uint32_t times, uint32_t burst_size)
{
    int32_t i;
    int32_t ret;
    ktime_t start_time, last_time;
    ktime_t trans_time;
    uint64_t trans_us;
    uint64_t trans_size, us_to_s, total_size;
    uint32_t size, copy_size, remainder;
    pci_addr_map addr_map_start, addr_map_end;
    void *pst_burst_buf = NULL;
    oal_pci_dev_stru *pst_pci_dev = NULL;
    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    if (burst_size > length) {
        pci_print_log(PCI_LOG_ERR, "burst_size  large %u than length %u", burst_size, length);
        return -OAL_EINVAL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pcie_res, cpu_address, &addr_map_start);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "invalid device cpu address 0x%x", cpu_address);
        return -OAL_EINVAL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pcie_res, cpu_address + length - 1, &addr_map_end);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "invalid device cpu address 0x%x and length %u", cpu_address, length);
        return -OAL_EINVAL;
    }

    pst_burst_buf = oal_memalloc(length);
    if (pst_burst_buf == NULL) {
        pci_print_log(PCI_LOG_ERR, "alloc burst buf failed, buf size:%u", length);
        return -OAL_ENOMEM;
    }

    trans_size = 0;
    start_time = ktime_get();
    for (i = 0; i < times; i++) {
        size = 0;
        remainder = length;
        forever_loop() {
            if (remainder == 0) {
                break;
            }

            copy_size = (remainder <= burst_size) ? remainder : burst_size;
            oal_pcie_io_trans((uintptr_t)pst_burst_buf, addr_map_start.va + size, copy_size);
            remainder -= copy_size;
            size += copy_size;
            trans_size += copy_size;
        }
    }
    total_size = trans_size;
    last_time = ktime_get();
    trans_time = ktime_sub(last_time, start_time);
    trans_us = (uint64_t)ktime_to_us(trans_time);
    if (trans_us == 0) {
        trans_us = 1;
    }

    trans_size = trans_size * 1000u;
    trans_size = trans_size * 1000u;
    trans_size = (trans_size >> PCIE_TRANS_US_OFFSET_BITS);
    trans_size = div_u64(trans_size, trans_us);
    us_to_s = trans_us;
    do_div(us_to_s, 1000000u);

    oal_io_print("red length:%u, total_size:%llu, burst_size:%u, thoughtput:%llu Mbps, trans_time:%llu us,  %llu s\n",
                 length, total_size, burst_size, trans_size, trans_us, us_to_s);

    oal_free(pst_burst_buf);

    return ret;
}

OAL_STATIC int32_t oal_pcie_performance_netbuf_alloc(oal_pcie_res *pst_pcie_res, uint32_t test_time)
{
    uint64_t count;
    int32_t ret = OAL_SUCC;
    oal_netbuf_stru *pst_netbuf = NULL;
    dma_addr_t pci_dma_addr;
    ktime_t start_time, last_time;
    ktime_t trans_time;
    unsigned long timeout;
    uint64_t trans_us;
    uint64_t trans_size, us_to_s, total_size;
    oal_pci_dev_stru *pst_pci_dev = NULL;
    pcie_cb_dma_res *pst_cb_res = NULL;
    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    start_time = ktime_get();
    timeout = jiffies + oal_msecs_to_jiffies(test_time);
    count = 0;
    forever_loop() {
        if (oal_time_after(jiffies, timeout)) {
            break;
        }

        pst_netbuf = oal_pcie_rx_netbuf_alloc(HCC_HDR_TOTAL_LEN + PCIE_EDMA_TRANS_MAX_FRAME_LEN, GFP_ATOMIC);
        if (pst_netbuf == NULL) {
            pci_print_log(PCI_LOG_INFO, "alloc netbuf failed!");
            break;
        }

        oal_netbuf_put(pst_netbuf, (HCC_HDR_TOTAL_LEN + PCIE_EDMA_TRANS_MAX_FRAME_LEN));

        pci_dma_addr = dma_map_single(&pst_pci_dev->dev, oal_netbuf_data(pst_netbuf),
                                      oal_netbuf_len(pst_netbuf), PCI_DMA_FROMDEVICE);
        if (dma_mapping_error(&pst_pci_dev->dev, pci_dma_addr)) {
            pci_print_log(PCI_LOG_INFO, "rx dma map netbuf failed, len=%u",
                          oal_netbuf_len(pst_netbuf));
            oal_netbuf_free(pst_netbuf);
            break;
        }

        pst_cb_res = (pcie_cb_dma_res *)oal_netbuf_cb(pst_netbuf);
        pst_cb_res->paddr.addr = pci_dma_addr;
        pst_cb_res->len = oal_netbuf_len(pst_netbuf);

        /* 释放内存 */
        dma_unmap_single(&pst_pci_dev->dev, (dma_addr_t)pst_cb_res->paddr.addr, pst_cb_res->len, PCI_DMA_FROMDEVICE);
        oal_netbuf_free(pst_netbuf);

        count++;
    }

    total_size = (HCC_HDR_TOTAL_LEN + PCIE_EDMA_TRANS_MAX_FRAME_LEN) * count;
    trans_size = total_size;
    last_time = ktime_get();
    trans_time = ktime_sub(last_time, start_time);
    trans_us = (uint64_t)ktime_to_us(trans_time);
    if (trans_us == 0) {
        trans_us = 1;
    }

    trans_size = trans_size * 1000u;
    trans_size = trans_size * 1000u;
    trans_size = (trans_size >> PCIE_TRANS_US_OFFSET_BITS);
    trans_size = div_u64(trans_size, trans_us);
    us_to_s = trans_us;
    do_div(us_to_s, 1000000u);

    oal_io_print("total_size:%llu, thoughtput:%llu Mbps, trans_time:%llu us,  %llu s\n",
                 total_size, trans_size, trans_us, us_to_s);

    return ret;
}

OAL_STATIC int32_t oal_pcie_performance_netbuf_queue(oal_pcie_res *pst_pcie_res,
                                                     uint32_t test_time, uint32_t alloc_count)
{
    uint64_t count;
    int32_t i;
    int32_t ret = OAL_SUCC;
    oal_netbuf_stru *pst_netbuf = NULL;
    oal_netbuf_head_stru st_netbuf_queue;
    dma_addr_t pci_dma_addr;
    ktime_t start_time, last_time;
    ktime_t trans_time;
    unsigned long timeout;
    uint64_t trans_us;
    uint64_t trans_size, us_to_s, total_size;
    oal_pci_dev_stru *pst_pci_dev = NULL;
    pcie_cb_dma_res *pst_cb_res = NULL;
    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    if (alloc_count == 0) {
        alloc_count = 1;
    }

    start_time = ktime_get();
    timeout = jiffies + oal_msecs_to_jiffies(test_time);
    count = 0;
    oal_netbuf_head_init(&st_netbuf_queue);
    forever_loop() {
        if (oal_time_after(jiffies, timeout)) {
            break;
        }

        for (i = 0; i < alloc_count; i++) {
            pst_netbuf = oal_pcie_rx_netbuf_alloc(HCC_HDR_TOTAL_LEN + PCIE_EDMA_TRANS_MAX_FRAME_LEN, GFP_ATOMIC);
            if (pst_netbuf == NULL) {
                pci_print_log(PCI_LOG_INFO, "alloc netbuf failed!");
                break;
            }

            oal_netbuf_put(pst_netbuf, (HCC_HDR_TOTAL_LEN + PCIE_EDMA_TRANS_MAX_FRAME_LEN));

            pci_dma_addr = dma_map_single(&pst_pci_dev->dev, oal_netbuf_data(pst_netbuf),
                                          oal_netbuf_len(pst_netbuf), PCI_DMA_FROMDEVICE);
            if (dma_mapping_error(&pst_pci_dev->dev, pci_dma_addr)) {
                pci_print_log(PCI_LOG_INFO, "rx dma map netbuf failed, len=%u",
                              oal_netbuf_len(pst_netbuf));
                oal_netbuf_free(pst_netbuf);
                break;
            }

            pst_cb_res = (pcie_cb_dma_res *)oal_netbuf_cb(pst_netbuf);
            pst_cb_res->paddr.addr = pci_dma_addr;
            pst_cb_res->len = oal_netbuf_len(pst_netbuf);

            oal_netbuf_list_tail_nolock(&st_netbuf_queue, pst_netbuf);
        }

        forever_loop() {
            pst_netbuf = oal_netbuf_delist_nolock(&st_netbuf_queue);
            if (pst_netbuf == NULL) {
                break;
            }

            pst_cb_res = (pcie_cb_dma_res *)oal_netbuf_cb(pst_netbuf);
            /* 释放内存 */
            dma_unmap_single(&pst_pci_dev->dev, (dma_addr_t)pst_cb_res->paddr.addr,
                             pst_cb_res->len, PCI_DMA_FROMDEVICE);
            oal_netbuf_free(pst_netbuf);
            count++;
        }
    }

    total_size = (HCC_HDR_TOTAL_LEN + PCIE_EDMA_TRANS_MAX_FRAME_LEN) * count;
    last_time = ktime_get();
    trans_time = ktime_sub(last_time, start_time);
    trans_us = (uint64_t)ktime_to_us(trans_time);
    if (trans_us == 0) {
        trans_us = 1;
    }
    trans_size = total_size;
    trans_size = trans_size * 1000u;
    trans_size = trans_size * 1000u;
    trans_size = (trans_size >> PCIE_TRANS_US_OFFSET_BITS);
    trans_size = div_u64(trans_size, trans_us);
    us_to_s = trans_us;
    do_div(us_to_s, 1000000u);

    oal_io_print("alloc_count:%u, thoughtput:%llu Mbps, trans_time:%llu us,  %llu s\n",
                 alloc_count, trans_size, trans_us, us_to_s);

    return ret;
}

OAL_STATIC int32_t oal_pcie_performance_cpu(oal_pcie_res *pst_pcie_res,
                                            uint32_t length, uint32_t times, uint32_t burst_size)
{
    int32_t i;
    int32_t ret = OAL_SUCC;
    ktime_t start_time, last_time;
    ktime_t trans_time;
    uint64_t trans_us;
    uint64_t trans_size, us_to_s, total_size;
    uint32_t size, copy_size, remainder;
    void *pst_burst_buf_src = NULL;
    void *pst_burst_buf_dst = NULL;
    oal_pci_dev_stru *pst_pci_dev = NULL;
    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    if (burst_size > length) {
        pci_print_log(PCI_LOG_ERR, "burst_size  large %u than length %u", burst_size, length);
        return -OAL_EINVAL;
    }

    pst_burst_buf_dst = oal_memalloc(length);
    if (pst_burst_buf_dst == NULL) {
        pci_print_log(PCI_LOG_ERR, "alloc src burst buf failed, buf size:%u", length);
        return -OAL_ENOMEM;
    }

    pst_burst_buf_src = oal_memalloc(length);
    if (pst_burst_buf_src == NULL) {
        pci_print_log(PCI_LOG_ERR, "alloc dst burst buf failed, buf size:%u", length);
        oal_free(pst_burst_buf_dst);
        return -OAL_ENOMEM;
    }

    trans_size = 0;
    start_time = ktime_get();
    for (i = 0; i < times; i++) {
        size = 0;
        remainder = length;
        forever_loop() {
            if (remainder == 0) {
                break;
            }

            copy_size = (remainder <= burst_size) ? remainder : burst_size;

            if (memcpy_s(pst_burst_buf_dst, length, pst_burst_buf_src, copy_size) != EOK) {
                pci_print_log(PCI_LOG_ERR, "memcpy_s error, destlen=%u, srclen=%u\n ", length, copy_size);
                break;
            }

            remainder -= copy_size;
            size += copy_size;
            trans_size += copy_size;
        }
    }
    total_size = trans_size;
    last_time = ktime_get();
    trans_time = ktime_sub(last_time, start_time);
    trans_us = (uint64_t)ktime_to_us(trans_time);
    if (trans_us == 0) {
        trans_us = 1;
    }

    trans_size = trans_size * 1000u;
    trans_size = trans_size * 1000u;
    trans_size = (trans_size >> PCIE_TRANS_US_OFFSET_BITS);
    trans_size = div_u64(trans_size, trans_us);
    us_to_s = trans_us;
    do_div(us_to_s, 1000000u);

    oal_io_print("red length:%u, total_size:%llu, burst_size:%u, thoughtput:%llu Mbps, trans_time:%llu us,  %llu s\n",
                 length, total_size, burst_size, trans_size, trans_us, us_to_s);

    oal_free(pst_burst_buf_dst);
    oal_free(pst_burst_buf_src);

    return ret;
}

OAL_STATIC int32_t oal_pcie_performance_write(oal_pcie_res *pst_pcie_res, uint32_t cpu_address, uint32_t length,
                                              uint32_t times, uint32_t burst_size, uint32_t value)
{
    int32_t i;
    int32_t ret;
    ktime_t start_time, last_time;
    ktime_t trans_time;
    uint64_t trans_us;
    uint64_t trans_size, us_to_s, total_size;
    uint32_t size, copy_size, remainder;
    pci_addr_map addr_map_start, addr_map_end;
    void *pst_burst_buf = NULL;
    oal_pci_dev_stru *pst_pci_dev = NULL;
    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    if (burst_size > length) {
        pci_print_log(PCI_LOG_ERR, "burst_size  large %u than length %u", burst_size, length);
        return -OAL_EINVAL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pcie_res, cpu_address, &addr_map_start);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "invalid device cpu address 0x%x", cpu_address);
        return -OAL_EINVAL;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pcie_res, cpu_address + length - 1, &addr_map_end);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "invalid device cpu address 0x%x and length %u", cpu_address, length);
        return -OAL_EINVAL;
    }

    pst_burst_buf = oal_memalloc(length);
    if (pst_burst_buf == NULL) {
        pci_print_log(PCI_LOG_ERR, "alloc burst buf failed, buf size:%u", length);
        return -OAL_ENOMEM;
    }

    memset_s(pst_burst_buf, length, (int32_t)value, length);

    trans_size = 0;
    start_time = ktime_get();
    for (i = 0; i < times; i++) {
        size = 0;
        remainder = length;
        forever_loop() {
            if (remainder == 0) {
                break;
            }

            copy_size = (remainder <= burst_size) ? remainder : burst_size;
            oal_pcie_io_trans(addr_map_start.va + size, (uintptr_t)pst_burst_buf, copy_size);
            remainder -= copy_size;
            size += copy_size;
            trans_size += copy_size;
        }
    }

    total_size = trans_size;
    last_time = ktime_get();
    trans_time = ktime_sub(last_time, start_time);
    trans_us = (uint64_t)ktime_to_us(trans_time);
    if (trans_us == 0) {
        trans_us = 1;
    }

    trans_size = trans_size * 1000u;
    trans_size = trans_size * 1000u;
    trans_size = (trans_size >> PCIE_TRANS_US_OFFSET_BITS);
    trans_size = div_u64(trans_size, trans_us);
    us_to_s = trans_us;
    do_div(us_to_s, 1000000u);

    oal_io_print(" write length:%u, total_size:%llu,burst_size:%u,value:0x%x thoughtput:%llu Mbps, \
                 trans_time:%llu us,  %llu s\n",
                 length, total_size, burst_size, value, trans_size, trans_us, us_to_s);

    oal_free(pst_burst_buf);

    return ret;
}

OAL_STATIC int32_t oal_pcie_debug_performance_read(struct kobject *dev, struct kobj_attribute *attr,
                                                   const char *buf, oal_pcie_res *pst_pcie_res)
{
    /* echo readmem address length(hex) > debug */
    int32_t ret;
    uint32_t cpu_address;
    uint32_t length, times, burst_size;

    /* Just for debug */
    if ((sscanf_s(buf, "0x%x %u %u %u", &cpu_address, &length, &times, &burst_size) != 4)) {
        return -OAL_EINVAL;
    }

    ret = oal_pcie_performance_read(pst_pcie_res, cpu_address, length, times, burst_size);
    return ret;
}

OAL_STATIC int32_t oal_pcie_debug_performance_cpu(struct kobject *dev, struct kobj_attribute *attr,
                                                  const char *buf, oal_pcie_res *pst_pcie_res)
{
    int32_t ret;
    uint32_t length, times, burst_size;

    /* Just for debug */
    if ((sscanf_s(buf, "%u %u %u", &length, &times, &burst_size) != 3)) {
        return -OAL_EINVAL;
    }

    ret = oal_pcie_performance_cpu(pst_pcie_res, length, times, burst_size);
    return ret;
}

OAL_STATIC int32_t oal_pcie_debug_performance_netbuf_alloc(struct kobject *dev, struct kobj_attribute *attr,
                                                           const char *buf, oal_pcie_res *pst_pcie_res)
{
    int32_t ret;
    uint32_t test_time;

    /* Just for debug */
    if ((sscanf_s(buf, "%u", &test_time) != 1)) {
        return -OAL_EINVAL;
    }

    ret = oal_pcie_performance_netbuf_alloc(pst_pcie_res, test_time);
    return ret;
}

OAL_STATIC int32_t oal_pcie_debug_performance_netbuf_queue(struct kobject *dev, struct kobj_attribute *attr,
                                                           const char *buf, oal_pcie_res *pst_pcie_res)
{
    int32_t ret;
    uint32_t test_time, alloc_count;

    /* Just for debug */
    if ((sscanf_s(buf, "%u %u", &test_time, &alloc_count) != 2)) {
        return -OAL_EINVAL;
    }

    ret = oal_pcie_performance_netbuf_queue(pst_pcie_res, test_time, alloc_count);
    return ret;
}

OAL_STATIC int32_t oal_pcie_debug_performance_write(struct kobject *dev, struct kobj_attribute *attr,
                                                    const char *buf, oal_pcie_res *pst_pcie_res)
{
    /* echo readmem address length(hex) > debug */
    int32_t ret;
    uint32_t cpu_address;
    uint32_t length, times, burst_size, value;

    /* Just for debug */
    if ((sscanf_s(buf, "0x%x %u %u %u 0x%x", &cpu_address, &length, &times, &burst_size, &value) != 5)) {
        return -OAL_EINVAL;
    }

    ret = oal_pcie_performance_write(pst_pcie_res, cpu_address, length, times, burst_size, value);
    return ret;
}

OAL_STATIC int32_t oal_pcie_loadfile(oal_pcie_res *pst_pcie_res, char *file_name,
                                     uint32_t cpu_address, int32_t performance)
{
    /* echo loadfile address filename > debug */
    /* echo loadfile 0x600000 /tmp/readmem.bin */
    struct file *fp = NULL;

    int32_t ret, rlen, total_len;
    pci_addr_map addr_map;

    void *pst_buf = NULL;
    oal_pci_dev_stru *pst_pci_dev = NULL;
    ktime_t start_time, last_time;
    ktime_t trans_time;
    uint64_t us_to_s, trans_us, file_us;
    mm_segment_t fs;

    if (pst_pcie_res == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pcie_res is null");
        return -OAL_EBUSY;
    }

    pst_pci_dev = pcie_res_to_dev(pst_pcie_res);
    if (pst_pci_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_pci_dev is null");
        return -OAL_ENODEV;
    }

    pst_buf = oal_memalloc(PAGE_SIZE);
    if (pst_buf == NULL) {
        pci_print_log(PCI_LOG_ERR, "pst_buf is null");
        return -OAL_ENOMEM;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);
    fp = filp_open(file_name, O_RDWR, 0664);
    set_fs(fs);
    if (oal_is_err_or_null(fp)) {
        pci_print_log(PCI_LOG_ERR, "open file error,fp = 0x%p, filename is [%s]\n", fp, file_name);
        oal_free(pst_buf);
        return -OAL_EINVAL;
    }

    pci_print_log(PCI_LOG_DBG, "loadfile cpu:0x%8x loadpath:%s", cpu_address, file_name);
    total_len = 0;
    file_us = 0;
    us_to_s = 0;
    trans_us = 0;

    if (performance) {
        start_time = ktime_get();
    }

    forever_loop() {
        rlen = oal_file_read_ext(fp, fp->f_pos, (void *)pst_buf, PAGE_SIZE);
        if (rlen <= 0) {
            break;
        }
        fp->f_pos += rlen;
        ret = oal_pcie_inbound_ca_to_va(pst_pcie_res, cpu_address + total_len, &addr_map);
        if (ret != OAL_SUCC) {
            pci_print_log(PCI_LOG_ERR, "loadfile address 0x%8x invalid", cpu_address + total_len);
            break;
        }
        total_len += rlen;
        oal_pcie_io_trans(addr_map.va, (uintptr_t)pst_buf, rlen);
    }

    if (performance) {
        last_time = ktime_get();
        trans_time = ktime_sub(last_time, start_time);
        trans_us = (uint64_t)ktime_to_us(trans_time);
        if (trans_us == 0) {
            trans_us = 1;
        }

        us_to_s = trans_us;
        do_div(us_to_s, 1000000u);
    }

    if (total_len) {
        if (performance)
            pci_print_log(PCI_LOG_INFO,
                          "loadfile cpu:0x%8x len:%u loadpath:%s done, cost %llu us %llu s, file_us:%llu",
                          cpu_address, total_len, file_name, trans_us, us_to_s, file_us);
        else
            pci_print_log(PCI_LOG_INFO, "loadfile cpu:0x%8x len:%u loadpath:%s done",
                          cpu_address, total_len, file_name);
    } else {
        pci_print_log(PCI_LOG_INFO, "loadfile cpu:0x%8x len:%u loadpath:%s failed",
                      cpu_address, total_len, file_name);
    }
    oal_free(pst_buf);
    oal_file_close(fp);
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_debug_loadfile(struct kobject *dev, struct kobj_attribute *attr,
                                           const char *buf, oal_pcie_res *pst_pcie_res)
{
    int32_t ret;
    char file_name[PCIE_DEBUG_MSG_LEN];
    uint32_t cpu_address;

    if (strlen(buf) >= sizeof(file_name)) {
        oal_io_print("input illegal!%s\n", __FUNCTION__);
        return -OAL_EINVAL;
    }

    if ((sscanf_s(buf, "0x%x %s", &cpu_address, file_name, sizeof(file_name)) != 2)) {
        pci_print_log(PCI_LOG_ERR,
                      "loadfile argument invalid,[%s], should be [echo writemem address  filename > debug]",
                      buf);
        return -OAL_EINVAL;
    }

    ret = oal_pcie_loadfile(pst_pcie_res, file_name, cpu_address, 1);

    return ret;
}

/* sysfs debug */
typedef struct _pcie_sysfs_debug_info_ {
    char *name;
    char *usage;
    int32_t (*debug)(struct kobject *dev, struct kobj_attribute *attr, const char *buf, oal_pcie_res *pst_pcie_res);
} pcie_sysfs_debug_info;

OAL_STATIC int32_t oal_pcie_print_debug_info(struct kobject *dev, struct kobj_attribute *attr,
                                             const char *buf, oal_pcie_res *pst_pcie_res)
{
    oal_pcie_print_debug_usages();
    return OAL_SUCC;
}

OAL_STATIC pcie_sysfs_debug_info g_pci_debug[] = {
    { "help",                     "",   oal_pcie_print_debug_info },
    { "dump_all_regions_mem",     "",   oal_pcie_dump_all_regions_mem },
    { "dump_all_regions_info",    "address(hex) value(hex)", oal_pcie_dump_all_regions },
    { "read32",                   "address(hex)",   oal_pcie_debug_read32 },
    { "write32",                  "address(hex) value(hex)", oal_pcie_debug_write32 },
    { "host_read32",              "address(hex)",  oal_pcie_debug_host_read32 },
    { "host_write32",             "address(hex) value(hex)",  oal_pcie_debug_host_write32 },
    { "read16",                   "address(hex)",    oal_pcie_debug_read16 },
    { "write16",                  "address(hex) value(hex)",  oal_pcie_debug_write16 },
    { "saveconfigmem",            "address(hex) length(decimal) filename", oal_pcie_debug_saveconfigmem },
    { "savemem",                  "address(hex) length(decimal) filename", oal_pcie_debug_savemem },
    { "save_hostmem",             "address(hex) length(decimal) filename", oal_pcie_debug_save_hostmem },
    { "loadfile",                 "address(hex) filename",  oal_pcie_debug_loadfile },
    { "readmem",                  "address(hex) length(decimal)", oal_pcie_debug_readmem },
    { "readconfigmem",            "address(hex) length(decimal)", oal_pcie_debug_readmem_config },
    { "performance_read",         "address(hex) length(decimal) times(decimal) burst_size(decimal)", oal_pcie_debug_performance_read },
    { "performance_cpu",          "length(decimal) times(decimal) burst_size(decimal)",                          oal_pcie_debug_performance_cpu },
    { "performance_write",        "address(hex) length(decimal) times(decimal) burst_size(decimal) value(hex) ", oal_pcie_debug_performance_write },
    { "performance_netbuf_alloc", "test_time(decimal msec)",  oal_pcie_debug_performance_netbuf_alloc },
    { "performance_netbuf_queue", "test_time(decimal msec) alloc_count(decimal)",  oal_pcie_debug_performance_netbuf_queue },
    { "testcase",          "casename(string)",                 oal_pcie_debug_testcase },
    { "wlan_power_on", "",                                 oal_pcie_debug_wlan_power_on },
    { "read_dsm32",        "dsm_type(decimal)",                oal_pcie_debug_read_dsm32 },
    { "write_dsm32",       "dsm_type(decimal) value(decimal)", oal_pcie_debug_write_dsm32 }
};

void oal_pcie_print_debug_usages(void)
{
    int32_t i;
    int32_t ret;
    void *buf = oal_memalloc(PAGE_SIZE);
    if (buf == NULL) {
        return;
    }

    for (i = 0; i < oal_array_size(g_pci_debug); i++) {
        ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "echo %s %s > /sys/hisys/pci/pcie/debug\n",
                         g_pci_debug[i].name ? : "", g_pci_debug[i].usage ? : "");
        if (ret < 0) {
            oal_free(buf);
            pci_print_log(PCI_LOG_ERR, "log str format err line[%d]\n", __LINE__);
            return;
        }
        printk("%s", (char *)buf);
    }

    oal_free(buf);
}

OAL_STATIC void oal_pcie_print_debug_usage(int32_t i)
{
    int32_t ret;
    void *buf = oal_memalloc(PAGE_SIZE);
    if (buf == NULL) {
        return;
    }

    ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "echo %s %s > /sys/hisys/pci/pcie/debug\n",
                     g_pci_debug[i].name ? : "", g_pci_debug[i].usage ? : "");
    if (ret < 0) {
        oal_free(buf);
        pci_print_log(PCI_LOG_ERR, "log str format err line[%d]\n", __LINE__);
        return;
    }
    printk("%s", (char *)buf);

    oal_free(buf);
}

OAL_STATIC ssize_t oal_pcie_get_debug_info(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret;
    int32_t i;
    int32_t count = 0;

    ret = snprintf_s(buf + count, PAGE_SIZE - count, PAGE_SIZE - count - 1, "pci debug cmds:\n");
    if (ret < 0) {
        pci_print_log(PCI_LOG_ERR, "log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;
    for (i = 0; i < oal_array_size(g_pci_debug); i++) {
        ret = snprintf_s(buf + count, PAGE_SIZE - count, PAGE_SIZE - count - 1, "%s\n", g_pci_debug[i].name);
        if (ret < 0) {
            pci_print_log(PCI_LOG_ERR, "log str format err line[%d]\n", __LINE__);
            return count;
        }
        count += ret;
    }
    return count;
}

OAL_STATIC ssize_t oal_pcie_set_debug_info(struct kobject *dev, struct kobj_attribute *attr,
                                           const char *buf, size_t count)
{
    int32_t i;
    oal_pcie_res *pst_pcie_res = oal_get_default_pcie_handler();

    if (buf[count] != '\0') { /* 确保传进来的buf是一个字符串, count不包含结束符 */
        pci_print_log(PCI_LOG_ERR, "invalid pci cmd\n");
        return 0;
    }

    for (i = 0; i < oal_array_size(g_pci_debug); i++) {
        if (g_pci_debug[i].name) {
            if ((count >= OAL_STRLEN(g_pci_debug[i].name)) &&
                !oal_memcmp(g_pci_debug[i].name, buf, OAL_STRLEN(g_pci_debug[i].name))) {
                /* 判断最后一个字符是回车还是空格 */
                char last_c = *(buf + OAL_STRLEN(g_pci_debug[i].name));
                if (last_c == '\n' || last_c == ' ' || last_c == '\0') {
                    break;
                }
            }
        }
    }

    if (i == oal_array_size(g_pci_debug)) {
        pci_print_log(PCI_LOG_ERR, "invalid pci cmd:%s\n", buf);
        oal_pcie_print_debug_usages();
        return count;
    }
#ifdef _PRE_COMMENT_CODE_
    oal_io_print("pcie cmd:%s process\n", g_pci_debug[i].name);
#endif
    buf += OAL_STRLEN(g_pci_debug[i].name);
    if (*buf != '\0') {  // count > OAL_STRLEN(g_pci_debug[i].name)
        buf += 1;        /* EOF */
    }

    for (; *buf == ' '; buf++);

    if (g_pci_debug[i].debug(dev, attr, (const char *)buf, pst_pcie_res) == -OAL_EINVAL) {
        oal_pcie_print_debug_usage(i);
    }
    return count;
}
OAL_STATIC struct kobj_attribute g_dev_attr_debug =
    __ATTR(debug, S_IRUGO | S_IWUSR, oal_pcie_get_debug_info, oal_pcie_set_debug_info);
OAL_STATIC struct attribute *g_hpci_sysfs_entries[] = {
    &g_dev_attr_debug.attr,
    NULL
};

OAL_STATIC struct attribute_group g_hpci_attribute_group = {
    .name = "pcie",
    .attrs = g_hpci_sysfs_entries,
};


int32_t oal_pcie_sysfs_group_create(oal_kobject *root_obj)
{
    return oal_debug_sysfs_create_group(root_obj, &g_hpci_attribute_group);
}

void oal_pcie_sysfs_group_remove(oal_kobject *root_obj)
{
    oal_debug_sysfs_remove_group(root_obj, &g_hpci_attribute_group);
}

#endif
