/*
 * This file is IVP driver related operations.
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

#ifndef _IVP_H_
#define _IVP_H_

#define SIZE_512K                        (512 * 1024)
#define SIZE_1MB                         (1 * 1024 * 1024)
#define SIZE_2MB                         (SIZE_1MB * 2)
#define MASK_1MB                         (SIZE_1MB - 1)
#define SIZE_4K                          (4 * 1024)
#define IVP_MMAP_SHIFT                   4
#define SHIFT_1MB                        20
#define IVP_STATUS_CHECK_DELAY_US        100

#define IVP_MODULE_NAME                  "dev-ivp"
#define IVP1_MODULE_NAME                 "dev-ivp1"
#define IVP_REGULATOR                    "dev-ivp"
#define IVP_MEDIA_REGULATOR              "dev-ivp-media"
#define IVP_SMMU_TCU_REGULATOR           "dev-ivp-smmu-tcu"

/* dts info */
#define OF_IVP_SECTION_NAME              "section_mem"
#define OF_IVP_SECTION_NODE_NAME         "ivp_mem_section"
#define OF_IVP_DDR_MEM_NAME              "ddr_mem"
#define OF_IVP_SHARE_MEM_NAME            "share_mem"
#define OF_IVP_LOG_MEM_NAME              "log_mem"

#define OF_IVP_CLK_NAME                  "dev-ivp-clk"
#define OF_IVP_CLK_RATE_NAME             "ivp-clk-rate"
#define OF_IVP_MIDDLE_CLK_RATE_NAME      "ivp-middle-clk-rate"
#define OF_IVP_LOW_CLK_RATE_NAME         "ivp-low-clk-rate"
#define OF_IVP_ULTRA_LOW_CLK_RATE_NAME   "ivp-ultra-low-clk-rate"
#define OF_IVP_LOW_CLK_PU_RATE_NAME      "ivp-lowfrq-pu-clk-rate"
#define OF_IVP_LOW_TEMP_RATE_NAME        "ivp-lowtemp-clk-rate"
#define OF_IVP_LOWFREQ_CLK_RATE_NAME     "lowfrq-pd-clk-rate"

#define OF_IVP_DYNAMIC_MEM               "ivp-dynamic-mem"
#define OF_IVP_DYNAMIC_MEM_SEC_SIZE      "ivp-dynamic-mem-section-size"
#define OF_IVP_SEC_SUPPORT               "ivp-sec-support-flag"
#define OF_IVP_PRIVATE_MEM               "ivp-private-mem"

/* define ioctl */
#define IVP_IOCTL_SECTCOUNT           _IOR('v', 0x70, unsigned int)
#define IVP_IOCTL_SECTINFO            _IOWR('v', 0x71, struct ivp_sect_info)
#define IVP_IOCTL_DSP_RUN             _IOW('v', 0x72, unsigned int)
#define IVP_IOCTL_DSP_SUSPEND         _IOW('v', 0x73, unsigned int)
#define IVP_IOCTL_DSP_RESUME          _IOW('v', 0x74, unsigned int)
#define IVP_IOCTL_DSP_STOP            _IOW('v', 0x75, unsigned int)
#define IVP_IOCTL_QUERY_RUNSTALL      _IOR('v', 0x76, unsigned int)
#define IVP_IOCTL_QUERY_WAITI         _IOR('v', 0x77, unsigned int)
#define IVP_IOCTL_TRIGGER_NMI         _IOW('v', 0x78, unsigned int)
#define IVP_IOCTL_WATCHDOG            _IOR('v', 0x79, unsigned int)
#define IVP_IOCTL_WATCHDOG_SLEEP      _IOR('v', 0x7A, unsigned int)
#define IVP_IOCTL_SMMU_INVALIDATE_TLB _IOW('v', 0x7B, unsigned int)
#define IVP_IOCTL_BM_INIT             _IOW('v', 0x7C, unsigned int)
#define IVP_IOCTL_CLK_LEVEL           _IOW('v', 0x7D, unsigned int)
#define IVP_IOCTL_POWER_UP            _IOW('v', 0x7E, struct ivp_power_up_info)
#define IVP_IOCTL_DUMP_DSP_STATUS     _IOW('v', 0x7F, unsigned int)
#define IVP_IOCTL_QUERY_CHIP_TYPE     _IOW('v', 0x80, unsigned int)
#define IVP_IOCTL_LOAD_FIRMWARE       _IOW('v', 0x8A, struct ivp_image_info)
#define IVP_IOCTL_POWER_DOWN          _IOW('v', 0x8F, unsigned int)
#define IVP_IOCTL_IPC_FLUSH_ENABLE    _IOWR('v', 0x60, unsigned int)
#define IVP_IOCTL_IPC_IVP_SECMODE     _IOWR('v', 0x61, unsigned int)
#define IVP_IOCTL_AVAILABLE_CORE      _IOR('v',  0x81, unsigned int)
#ifdef MULTIPLE_ALGO
#define IVP_IOCTL_LOAD_CORE           _IOW('v',  0x8B, struct ivp_image_info)
#define IVP_IOCTL_LOAD_ALGO           _IOW('v',  0x8C, struct ivp_image_info)
#define IVP_IOCTL_UNLOAD_ALGO         _IOW('v',  0x8D, unsigned int)
#define IVP_IOCTL_ALGOINFO            _IOWR('v',  0x8E, struct ivp_algo_info)
#endif
#define IVP_IOCTL_FDINFO              _IOWR('v',  0x90, struct ivp_fd_info)
#define IVP_IOCTL_HIDL_MAP            _IOWR('v',  0x91, struct ivp_map_info)
#define IVP_IOCTL_HIDL_UNMAP          _IOWR('v',  0x92, struct ivp_map_info)

enum ivp_core_id {
	IVP_CORE0_ID = 0,
	IVP_CORE1_ID,
	IVP_CORE_MAX_NUM
};

#define MAX_INDEX                        64
#define SECT_START_NUM                   3
#define MAX_NAME                         64
#define COMPAT_SIZE                      8

enum SEC_MODE {
	NOSEC_MODE = 0,
	SECURE_MODE = 1
};
enum IVP_SECTION_INDEX {
	DRAM0_SECTION_INDEX = 0,
	DRAM1_SECTION_INDEX,
	IRAM_SECTION_INDEX,
	DDR_SECTION_INDEX,
	SHARE_SECTION_INDEX,
	LOG_SECTION_INDEX,
	IVP_SECTION_MAX
};

struct ivp_algo_info {
	unsigned int algo_id;
	unsigned int algo_start; /* algo entry point address */
	unsigned int rel_addr;   /* algo relacation address */
	unsigned int rel_count;  /* algo relacation count */
	unsigned int ddr_addr;   /* algo ddr text address */
	unsigned int ddr_vaddr;  /* algo ddr virtual address */
	unsigned int ddr_size;   /* algo ddr text size */
	unsigned int func_addr;  /* algo hot function address */
	unsigned int func_vaddr; /* algo hot virtual address */
	unsigned int func_size;  /* algo hot function size */
	unsigned int data_addr;  /* algo data section address */
	unsigned int data_vaddr; /* algo data virtual address */
	unsigned int data_size;  /* algo data section size */
	unsigned int bss_addr;   /* algo bss section address */
	unsigned int bss_vaddr;  /* algo bss virtual address */
	unsigned int bss_size;   /* algo bss section size */
};

#ifdef CONFIG_IVP_SMMU_V3
enum IVP_SMMU_ID_INDEX {
	IVP_SMMU_SID = 0,
	IVP_SMMU_SSID,
	IVP_SMMU_ID_MAX
};
#endif

struct ivp_sect_info {
	char name[MAX_NAME];
	unsigned int index;
	unsigned int len;
	unsigned int ivp_addr;
	unsigned int info_type;
	union {
		unsigned long acpu_addr;
		char compat32[COMPAT_SIZE];
	};
};

struct ivp_image_info {
	char name[MAX_NAME];
	unsigned int length;
	int algo_id;
};

struct ivp_power_up_info {
	int sec_mode;
	int sec_buff_fd;
};

struct ion_page_info {
	unsigned long phys_addr;
	unsigned int npages;
};

struct sglist {
	unsigned long long sglist_size;
	unsigned long long ion_size;
	unsigned long long ion_id;
	unsigned long long info_length;
	struct ion_page_info page_info[0];
};

struct ivp_fd_info {
	int fd;
	unsigned int size;
	unsigned int sec_flag;
	unsigned int mapped;
	unsigned int iova;
};

struct ivp_map_info {
	int fd;
	unsigned long fd_size;
	unsigned long fd_prot;
	unsigned long iova;
	unsigned long mapped_size;
};

#endif /* _IVP_H_ */
