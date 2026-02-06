/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: define the common interface for Boot Detector and Boot Recovery
 * Author: qidechun
 * Create: 2019-03-05
 */

#ifndef ADAPTER_H
#define ADAPTER_H

/* ----includes ---- */
/* ----c++ support ---- */
#ifdef __cplusplus
extern "C" {
#endif

/* ---- export macroes ---- */
/* ---- export prototypes ---- */
struct adapter {
	struct {
		char *dev_path;
		unsigned int part_size;
		struct {
		int (*read)(const char *dev_path,
			unsigned long long offset,
			char *buf,
			unsigned long long buf_size);
		int (*write)(const char *dev_path,
			unsigned long long offset,
			const char *buf,
			unsigned long long buf_size);
		} ops;
	} bfi_part;
	struct {
		unsigned long base;
		unsigned long size;
		struct {
			int (*read)(unsigned long dst,
				unsigned long dst_max,
				void *phys_mem_addr,
				unsigned long data_len);
			int (*write)(unsigned long dst,
				unsigned long dst_max,
				void *src,
				unsigned long src_len);
		} ops;
	} pyhs_mem_info;
	struct {
		int (*memset_s)(void *dst, unsigned long dst_max,
			int c, unsigned long count);
		int (*memcpy_s)(void *dst, unsigned long dst_max,
			const void *src, unsigned long count);
		int (*strncpy_s)(char *dst, unsigned long dst_max,
			const char *src, unsigned long count);
		int (*memcmp)(const void *dst,
			const void *src, unsigned long count);
		unsigned long (*strlen)(const char *s);
	} sec_funcs;
	struct {
		void* (*malloc)(unsigned long size);
		void (*free)(const void *p);
	} mem_ops;
	struct {
		void (*lock)(void);
		void (*trylock)(void);
		void (*unlock)(void);
	} mutex_ops;
	struct {
		void (*reboot)(void);
		void (*shutdown)(void);
	} sys_ctl;
	struct {
		void (*degrade)(int excp_type);
		void (*bypass)(int excp_type);
		void (*load_backup)(const char *part_name);
	} prevent;
	struct {
		char log_name[16]; /* max file name length */
		unsigned int log_size;
		void (*capture_bl_log)(char *pbuf,
			unsigned int buf_size);
	} bl_log_ops;
	struct {
		char log_name[16]; /* max file name length */
		unsigned int log_size;
		void (*capture_kernel_log)(char *pbuf,
			unsigned int buf_size);
	} kernel_log_ops;
	struct {
		int (*set_stage)(int stage);
		int (*get_stage)(int *pstage);
	} stage_ops;
	void (*print)(const char *format, ...);
	void (*sha256)(unsigned char *pout, unsigned long out_len,
		const void *pin, unsigned long in_len);
	void (*notify_storage_fault)(unsigned long long bopd_mode);
	void (*save_misc_msg)(const char *pmsg);
	int (*is_boot_detector_enabled)(void);
	int (*is_boot_recovery_enabled)(void);
	int (*is_bopd_supported)(void);
	int (*is_safe_mode_supported)(void);
	int (*is_bootloader)(void);
};

/* ---- c++ support ---- */
#ifdef __cplusplus
}
#endif
#endif
